#include "widget.h"
#include "ui_widget.h"

TrafficGenerator::TrafficGenerator() {
}

void TrafficGenerator::setWorkStatus(bool workStatus) {

    if(!m_workStatus && workStatus) {
        flushStatistic();
        m_workStatus = true;
    }

    if(m_workStatus && !workStatus) {
        m_endTime = QDateTime::currentDateTime();
        m_workStatus = false;
    }
}

bool TrafficGenerator::getWorkStatus() const {
    return m_workStatus;
}

void TrafficGenerator::setCleanFilesDir(const QString& cleanFilesDir) {
    m_cleanFilesDir = cleanFilesDir;
}

QString TrafficGenerator::getCleanFilesDir() const {
    return m_cleanFilesDir;
}

void TrafficGenerator::setCleanFiles(const QFileInfoList& cleanFiles) {
    m_cleanFiles = cleanFiles;
}

QFileInfoList& TrafficGenerator::getCleanFiles() {
    return m_cleanFiles;
}

void TrafficGenerator::setInfectedFiles(const QFileInfoList& infectedFiles) {
    m_infectedFiles = infectedFiles;
}

QFileInfoList& TrafficGenerator::getInfectedFiles() {
    return m_infectedFiles;
}

void TrafficGenerator::setInfectedFilesDir(const QString& infectedFilesDir) {
    m_infectedFilesDir = infectedFilesDir;
}

QString TrafficGenerator::getInfectedFilesDir() const {
    return m_infectedFilesDir;
}

void TrafficGenerator::setDestinationDir(const QString& destinationDir) {
    m_destinationDir = destinationDir;
}

QString TrafficGenerator::getDestinationDir() const {
    return m_destinationDir;
}

void TrafficGenerator::setGenerateInterval(int generateInterval) {
    m_generateInterval = generateInterval;
}

int TrafficGenerator::getGenerateInterval() const {
    return m_generateInterval;
}

void TrafficGenerator::setFilesPerInterval(int filesPerInterval) {
    m_filesPerInterval = filesPerInterval;
}

int TrafficGenerator::getFilesPerInterval() const {
    return m_filesPerInterval;
}

void TrafficGenerator::setInfectedFileGenerateProbability(double infectedFileGenerateProbability) {
    m_infectedFileGenerateProbability = infectedFileGenerateProbability;
}

void TrafficGenerator::addCleanFiles(QStringList fileNames) {
    foreach(QString fileName, fileNames) {
        if(QFile(fileName).exists()) {
            m_cleanFiles << QFileInfo(fileName);
        }
    }
    if(m_cleanFiles.size()) {
        m_cleanFilesDir = m_cleanFiles.last().dir().path();
    }
}

void TrafficGenerator::addInfectedFiles(QStringList fileNames) {
    foreach(QString fileName, fileNames) {
        if(QFile(fileName).exists()) {
            m_infectedFiles << QFileInfo(fileName);
        }
    }
    if(m_infectedFiles.size()) {
        m_infectedFilesDir = m_infectedFiles.last().dir().path();
    }
}

double TrafficGenerator::getInfectedFileGenerateProbability() const {
    return m_infectedFileGenerateProbability;
}

double TrafficGenerator::getCurrentSpeedInBytes() const {
    return m_currentSpeedInBytes;
}

double TrafficGenerator::getAverageSpeedInBytes() const {
    return m_averageSpeedInBytes;
}

double TrafficGenerator::getTotalVolInBytes() const {
    return m_totalVolInBytes;
}

int TrafficGenerator::getGlobalCnt() const {
    return m_globalCnt;
}

int TrafficGenerator::getInfectedFilesNb() const {
    return m_infectedFilesCnt;
}

void TrafficGenerator::flushStatistic() {
    m_totalVolInBytes = 0.;
    m_averageSpeedInBytes = 0;
    m_globalCnt = 0;
    m_infectedFilesCnt = 0;
    m_startTime = QDateTime::currentDateTime();
    m_endTime = QDateTime::currentDateTime();
}

qint64 TrafficGenerator::getWorkTimeInSecs() {
    if(m_workStatus)
        m_endTime = QDateTime::currentDateTime();
    return m_startTime.secsTo(m_endTime);
}

QDateTime TrafficGenerator::getStartTime() const {
    return m_startTime;
}

QDateTime TrafficGenerator::getEndTime() const {
    return m_endTime;
}

void TrafficGenerator::start() {
    flushStatistic();
    m_workStatus = true;
    generate();
}

void TrafficGenerator::generate() {

    if(m_workStatus) {

        if(!m_cleanFiles.size() && !m_infectedFiles.size()) {
            emit executeError(-1);
            stop();
        } else {
            for(int i = 0; i < m_filesPerInterval; i++) {
                generateFile(QRandomGenerator::global()->generateDouble() < m_infectedFileGenerateProbability ?
                                 m_infectedFiles : m_cleanFiles);
                emit updateProgressBar((i + 1) * 100 / m_filesPerInterval);
            }
            emit updateUi();
        }
    }
}

void TrafficGenerator::generateFile(QFileInfoList& sourceList) {
    if(sourceList.size()) {
        double volumeInBytes{0};

        int randIdx = int(QRandomGenerator::global()->bounded(sourceList.size()));
        QFile::copy(sourceList.at(randIdx).absoluteFilePath(),
                       QString("%1/%2_%3").arg(m_destinationDir).
                                           arg(QString::number(m_globalCnt)).
                                           arg(sourceList.at(randIdx).fileName()));
        volumeInBytes += sourceList.at(randIdx).size();
        m_globalCnt++;

        if(m_infectedFiles.contains(sourceList.at(randIdx))) {
            m_infectedFilesCnt++;
        }

        m_currentSpeedInBytes = volumeInBytes / m_filesPerInterval;
        m_totalVolInBytes += volumeInBytes;
        m_averageSpeedInBytes = m_totalVolInBytes / getWorkTimeInSecs();
    }
}

void TrafficGenerator::stop() {
    m_endTime = QDateTime::currentDateTime();
    m_workStatus = false;
    emit updateUi();
}

// -----------------------------------------------------------------------------------------

Widget::Widget(QWidget *parent): QWidget(parent), ui(new Ui::Widget), settings("FeZar97", "TrafficGenerator"){
    ui->setupUi(this);
    setLayout(ui->mainLayout);
    setWindowTitle(QString("Traffic Generator ") + VERSION);

    m_updateTimer.setInterval(1000);
    connect(&m_updateTimer, &QTimer::timeout, this, &Widget::updateUi);
    m_updateTimer.start();

    connect(&m_generateTimer, &QTimer::timeout, &trfGen, &TrafficGenerator::generate);

    prepareTables();

    connect(&trfGen, &TrafficGenerator::updateProgressBar,  this->ui->generationProgressBar, &QProgressBar::setValue);
    connect(&trfGen, &TrafficGenerator::updateUi,           this,                            &Widget::updateUi);
    connect(&trfGen, &TrafficGenerator::executeError,       this,                            &Widget::executeError);

    restoreGeometry(settings.value("geometry").toByteArray());
    setGenerationInterval(settings.value("generateInterval",      DEFAULT_GENERATE_INTERVAL).toInt());
    trfGen.setDestinationDir(settings.value("destinationDir",     QDir::tempPath()).toString());

    trfGen.setCleanFilesDir(settings.value("cleanFilesDir",       QDir::tempPath()).toString());
    trfGen.setInfectedFilesDir(settings.value("infectedFilesDir", QDir::tempPath()).toString());

    trfGen.setInfectedFileGenerateProbability(settings.value("infectedFileProbability", DEFAULT_INFECTED_FILE_PROBABILITY).toDouble());

    trfGen.setFilesPerInterval(settings.value("filesPerInterval", DEFAULT_FILES_NB_PER_INTERVAL).toInt());

    ui->currentSpeedUnitCB->setCurrentIndex(settings.value("currentSpeedUnitIdx", DEFAULT_UNIT).toInt());
    ui->averageSpeedUnitCB->setCurrentIndex(settings.value("averageSpeedUnitIdx", DEFAULT_UNIT).toInt());
    ui->totalVolumeUnitCB->setCurrentIndex(settings.value("totalVolumeUnitIdx",   DEFAULT_UNIT).toInt());
    ui->infectedFileProbabilityDSB->setValue(trfGen.getInfectedFileGenerateProbability());

    trfGen.moveToThread(&trafficThread);
    trafficThread.start();

    updateUi();
}

Widget::~Widget() {
    settings.setValue("geometry", saveGeometry());

    settings.setValue("generateInterval",        trfGen.getGenerateInterval());
    settings.setValue("destinationDir",          trfGen.getDestinationDir());
    settings.setValue("cleanFilesDir",           trfGen.getCleanFilesDir());
    settings.setValue("infectedFilesDir",        trfGen.getInfectedFilesDir());
    settings.setValue("filesPerInterval",        trfGen.getFilesPerInterval());
    settings.setValue("infectedFileProbability", trfGen.getInfectedFileGenerateProbability());
    settings.setValue("currentSpeedUnitIdx",     ui->currentSpeedUnitCB->currentIndex());
    settings.setValue("averageSpeedUnitIdx",     ui->averageSpeedUnitCB->currentIndex());
    settings.setValue("totalVolumeUnitIdx",      ui->totalVolumeUnitCB->currentIndex());

    trafficThread.quit();
    trafficThread.wait();

    delete ui;
}

void Widget::updateUi() {
    m_endDateTime = trfGen.getWorkStatus() ? QDateTime::currentDateTime() : m_endDateTime;

    ui->startButton->setEnabled(!trfGen.getWorkStatus());
    ui->stopButton->setEnabled(trfGen.getWorkStatus());

    ui->currentSpeedInfoLabel->setText(QString::number(convert(trfGen.getCurrentSpeedInBytes(),
                                                               ui->currentSpeedUnitCB->currentIndex())));
    ui->averageSpeedInfoLabel->setText(QString::number(convert(trfGen.getAverageSpeedInBytes(),
                                                               ui->averageSpeedUnitCB->currentIndex())));

    ui->virusNbInfoLabel->setText(QString::number(trfGen.getInfectedFilesNb()));
    ui->destinationDirLE->setText(trfGen.getDestinationDir());
    ui->generationFileNbSB->setValue(trfGen.getFilesPerInterval());
    ui->generationIntervalSB->setValue(trfGen.getGenerateInterval());
    ui->generationProgressInfoLabel->setText(QString::number(trfGen.getGlobalCnt()));

    qint64 days;
    if( QDate(trfGen.getStartTime().date()).daysTo(trfGen.getEndTime().date()) == 0 ) {
        days = 0;
    } else {
        if(trfGen.getEndTime().time() >= trfGen.getStartTime().time()) {
            days = QDate(trfGen.getStartTime().date()).daysTo(trfGen.getEndTime().date());
        } else {
            days = QDate(trfGen.getStartTime().date()).daysTo(trfGen.getEndTime().date()) - 1;
        }
    }

    ui->workTimeInfoLabel->setText(QString("%1 дней ").arg( days ) +
                                   QTime(0,0,0,0).addSecs(QTime(trfGen.getStartTime().time()).secsTo(trfGen.getEndTime().time())).toString("hh ч. mm мин. ss сек."));


    ui->totalVolumeInfoLabel->setText(QString::number(convert(trfGen.getTotalVolInBytes(),
                                                              ui->totalVolumeUnitCB->currentIndex())));

    for(int i = 0; i < trfGen.getCleanFiles().size(); i++) {
        m_cleanFilesModel.setItem(i, 0, new QStandardItem(trfGen.getCleanFiles().at(i).fileName()));
        m_cleanFilesModel.setItem(i, 1, new QStandardItem(QString::number(trfGen.getCleanFiles().at(i).size() / (1024. * 1024.), 'f', 2)));
    }

    for(int i = 0; i < trfGen.getInfectedFiles().size(); i++) {
        m_infectedFilesModel.setItem(i, 0, new QStandardItem(trfGen.getInfectedFiles().at(i).fileName()));
        m_infectedFilesModel.setItem(i, 1, new QStandardItem(QString::number(trfGen.getInfectedFiles().at(i).size() / (1024. * 1024.), 'f', 2)));
    }
}

void Widget::prepareTables() {

    m_cleanFilesModel.setHorizontalHeaderLabels(QStringList() << "Файл" << "Размер\n(Мбайт)");
    ui->cleanFilesTableView->setModel(&m_cleanFilesModel);

    ui->cleanFilesTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->cleanFilesTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->cleanFilesTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->cleanFilesTableView->setSelectionMode(QAbstractItemView::NoSelection);

    m_infectedFilesModel.setHorizontalHeaderLabels(QStringList() << "Файл" << "Размер\n(Мбайт)");
    ui->infectedFilesTableView->setModel(&m_infectedFilesModel);

    ui->infectedFilesTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->infectedFilesTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->infectedFilesTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->infectedFilesTableView->setSelectionMode(QAbstractItemView::NoSelection);
}

void Widget::setGenerationInterval(int secs) {
    m_generateTimer.setInterval(secs * 1000);
    trfGen.setGenerateInterval(secs);
}

void Widget::executeError(int code) {
    on_stopButton_clicked();
    switch(code) {
        case -1:
            QMessageBox::critical(nullptr, "Ошибка", "Не выбраны файлы-шаблоны для генерации!", QMessageBox::Ok, QMessageBox::Ok);
            break;
        default:
            break;
    }
}

void Widget::on_generationFileNbSB_valueChanged(int newVal) {
    trfGen.setFilesPerInterval(newVal);
}

void Widget::on_generationIntervalSB_valueChanged(int newVal) {
    setGenerationInterval(newVal);
}

void Widget::on_destinationDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    QString("Выбор папки назначения"),
                                                    trfGen.getDestinationDir());
    trfGen.setDestinationDir(dir);
}

void Widget::on_startButton_clicked() {
    trfGen.start();
    m_startDateTime = QDateTime::currentDateTime();
    m_generateTimer.start();
}

void Widget::on_stopButton_clicked() {
    trfGen.stop();
    m_generateTimer.stop();
}

void Widget::on_cleanFilesAddButton_clicked() {
    trfGen.addCleanFiles(QFileDialog::getOpenFileNames(this,
                                                       "Выбор файлов-образцов",
                                                       trfGen.getCleanFilesDir(),
                                                       "*.*"));
    updateUi();
}

void Widget::on_cleanFilesClearButton_clicked() {
    trfGen.getCleanFiles().clear();
    if(m_cleanFilesModel.rowCount() > 0)
        m_cleanFilesModel.removeRows(0, m_cleanFilesModel.rowCount());
    updateUi();
}

void Widget::on_infectedFilesAddButton_clicked() {
    trfGen.addInfectedFiles(QFileDialog::getOpenFileNames(this,
                                                          "Выбор файлов-образцов",
                                                          trfGen.getInfectedFilesDir(),
                                                          "*.*"));
    updateUi();
}

void Widget::on_infectedFilesClearButton_clicked() {
    trfGen.getInfectedFiles().clear();
    if(m_infectedFilesModel.rowCount() > 1)
        m_infectedFilesModel.removeRows(0, m_infectedFilesModel.rowCount());
    updateUi();
}

void Widget::on_currentSpeedUnitCB_currentIndexChanged(int index) {
    Q_UNUSED(index)
    updateUi();
}

void Widget::on_averageSpeedUnitCB_currentIndexChanged(int index) {
    Q_UNUSED(index)
    updateUi();
}

void Widget::on_totalVolumeUnitCB_currentIndexChanged(int index) {
    Q_UNUSED(index)
    updateUi();
}

void Widget::on_infectedFileProbabilityDSB_valueChanged(double probability) {
    trfGen.setInfectedFileGenerateProbability(probability);
}

double convert(double bytes, int outType) {
    switch(outType) {
        case 0: // bits
            return bytes * 8;
        case 1: // bytes
            return bytes;
        case 2: // kilobits
            return bytes * 8. / 1024.;
        case 3: // kilobytes
            return bytes / 1024.;
        case 4: // megabits
            return bytes * 8. / 1024. / 1024.;
        case 5: // megabytes
            return bytes / 1024. / 1024.;
        case 6: // gigabits
            return bytes * 8. / 1024. / 1024. / 1024.;
        case 7: // gigabytes
            return bytes / 1024. / 1024. / 1024.;
        default:
            return 0;
    }
}
