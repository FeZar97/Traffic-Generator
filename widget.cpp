#include "widget.h"
#include "ui_widget.h"

int TrafficGenerator::globalCnt() const {
    return m_globalCnt;
}

int TrafficGenerator::getVirusNb() const {
    return m_virusNb;
}

void TrafficGenerator::flushStatistic() {
    m_totalVolInBytes = 0.;
    m_averageSpeedInBytes = 0;
    m_globalCnt = 0;
    m_virusNb = 0;
    m_startTime = QDateTime::currentDateTime();
    m_endTime = QDateTime::currentDateTime();
}

bool TrafficGenerator::getWorkStatus() const {
    return m_workStatus;
}

TrafficGenerator::TrafficGenerator() {}

void TrafficGenerator::addFilesAsSource(QStringList fileNames) {
    foreach(QString fileName, fileNames) {
        if(QFile(fileName).exists()) {
            m_sourceFiles << QFileInfo(fileName);
        }
    }
    if(fileNames.size())
        emit setTemplateDir(QFileInfo(fileNames.last()).dir().absolutePath());
}

QFileInfoList* TrafficGenerator::sourceFiles() {
    return &m_sourceFiles;
}

int TrafficGenerator::generateInterval() const {
    return m_generateInterval;
}

void TrafficGenerator::setGenerateInterval(int generateInterval) {
    m_generateInterval = generateInterval;
}

int TrafficGenerator::filesPerInterval() const {
    return m_filesPerInterval;
}

void TrafficGenerator::setFilesPerInterval(int filesPerInterval) {
    m_filesPerInterval = filesPerInterval;
}

QString TrafficGenerator::destinationDir() const {
    return m_destinationDir;
}

void TrafficGenerator::setDestinationDir(const QString& destinationDir) {
    m_destinationDir = destinationDir;
}

QString TrafficGenerator::getTemplateDir() const {
    return m_templateDir;
}

void TrafficGenerator::setTemplateDir(const QString& templateDir) {
    m_templateDir = templateDir;
}

double TrafficGenerator::currentSpeedInBytes() const {
    return m_currentSpeedInBytes;
}

double TrafficGenerator::averageSpeedInBytes() const {
    return m_averageSpeedInBytes;
}

double TrafficGenerator::totalVolInBytes() const {
    return m_totalVolInBytes;
}

void TrafficGenerator::setWorkStatus(bool workStatus) {

    if(!m_workStatus && workStatus) {
        m_globalCnt = 0;
        m_virusNb = 0;
        m_startTime = QDateTime::currentDateTime();
        m_workStatus = true;
    }

    if(m_workStatus && !workStatus) {
        m_endTime = QDateTime::currentDateTime();
        m_workStatus = false;
    }
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
        if(m_sourceFiles.size()) {
            double volumeInBytes{0};

            for(int i = 0; i < m_filesPerInterval; i++) {
                int randIdx = int(QRandomGenerator::global()->bounded(m_sourceFiles.size()));
                if(QFile::copy(m_sourceFiles.at(randIdx).absoluteFilePath(),
                               QString("%1/%2_%3").arg(m_destinationDir).
                                                   arg(QString::number(m_globalCnt)).
                                                   arg(m_sourceFiles.at(randIdx).fileName()))) {
                    volumeInBytes += m_sourceFiles.at(randIdx).size();
                    m_globalCnt++;

                    if(m_sourceFiles.at(randIdx).fileName() == "eicar.txt" || m_sourceFiles.at(randIdx).fileName() == "EICAR.zip") {
                        m_virusNb++;
                    }


                    emit updateProgressBar((i + 1) * 100 / m_filesPerInterval);
                }
            }

            m_currentSpeedInBytes = volumeInBytes / m_filesPerInterval;
            m_totalVolInBytes += volumeInBytes;
            m_averageSpeedInBytes = m_totalVolInBytes / getWorkTimeInSecs();
        } else {
            emit executeError(-1);
            stop();
        }
        emit updateUi();
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

    //ui->tabWidget->setLayout(ui->mainGenLayout);

    m_updateTimer.setInterval(1000);
    connect(&m_updateTimer, &QTimer::timeout, this, &Widget::updateUi);
    m_updateTimer.start();

    connect(&m_generateTimer, &QTimer::timeout, &trfGen, &TrafficGenerator::generate);

    m_model.setHorizontalHeaderLabels(QStringList() << "Файл" << "Размер\n(Мбайт)");
    ui->tableView->setModel(&m_model);

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setSelectionMode(QAbstractItemView::NoSelection);

    connect(&trfGen, &TrafficGenerator::updateProgressBar,  this->ui->generationProgressBar, &QProgressBar::setValue);
    connect(&trfGen, &TrafficGenerator::updateUi,           this,                            &Widget::updateUi);
    connect(&trfGen, &TrafficGenerator::executeError,       this,                            &Widget::executeError);

    restoreGeometry(settings.value("geometry").toByteArray());
    setGenerationInterval(settings.value("generateInterval", 3).toInt());
    trfGen.setDestinationDir(settings.value("destinationDir", QDir::tempPath()).toString());
    trfGen.setTemplateDir(settings.value("templateDir", QDir::tempPath()).toString());
    trfGen.setFilesPerInterval(settings.value("filesPerInterval", 5).toInt());
    ui->currentSpeedUnitCB->setCurrentIndex(settings.value("currentSpeedUnitIdx", 5).toInt());
    ui->averageSpeedUnitCB->setCurrentIndex(settings.value("averageSpeedUnitIdx", 5).toInt());
    ui->totalVolumeUnitCB->setCurrentIndex(settings.value("totalVolumeUnitIdx", 5).toInt());

    trfGen.moveToThread(&trafficThread);
    trafficThread.start();

    updateUi();
}

Widget::~Widget() {
    settings.setValue("geometry", saveGeometry());

    settings.setValue("generateInterval",    trfGen.generateInterval());
    settings.setValue("destinationDir",      trfGen.destinationDir());
    settings.setValue("templateDir",         trfGen.getTemplateDir());
    settings.setValue("filesPerInterval",    trfGen.filesPerInterval());
    settings.setValue("currentSpeedUnitIdx", ui->currentSpeedUnitCB->currentIndex());
    settings.setValue("averageSpeedUnitIdx", ui->averageSpeedUnitCB->currentIndex());
    settings.setValue("totalVolumeUnitIdx",  ui->totalVolumeUnitCB->currentIndex());

    trafficThread.quit();
    trafficThread.wait();

    delete ui;
}

void Widget::updateUi() {
    m_endDateTime = trfGen.getWorkStatus() ? QDateTime::currentDateTime() : m_endDateTime;

    ui->startButton->setEnabled(!trfGen.getWorkStatus());
    ui->stopButton->setEnabled(trfGen.getWorkStatus());

    ui->currentSpeedInfoLabel->setText(QString::number(convert(trfGen.currentSpeedInBytes(),
                                                               ui->currentSpeedUnitCB->currentIndex())));
    ui->averageSpeedInfoLabel->setText(QString::number(convert(trfGen.averageSpeedInBytes(),
                                                               ui->averageSpeedUnitCB->currentIndex())));

    ui->virusNbInfoLabel->setText(QString::number(trfGen.getVirusNb()));
    ui->destinationDirLE->setText(trfGen.destinationDir());
    ui->generationFileNbSB->setValue(trfGen.filesPerInterval());
    ui->generationIntervalSB->setValue(trfGen.generateInterval());
    ui->generationProgressInfoLabel->setText(QString::number(trfGen.globalCnt()));

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


    ui->totalVolumeInfoLabel->setText(QString::number(convert(trfGen.totalVolInBytes(),
                                                              ui->totalVolumeUnitCB->currentIndex())));

    for(int i = 0; i < trfGen.sourceFiles()->size(); i++) {
        m_model.setItem(i, 0, new QStandardItem(trfGen.sourceFiles()->at(i).fileName()));
        m_model.setItem(i, 1, new QStandardItem(QString::number(trfGen.sourceFiles()->at(i).size() / (1024. * 1024.), 'f', 2)));
    }
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
                                                    trfGen.destinationDir());
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

void Widget::on_sourceFilesClearButton_clicked() {
    trfGen.sourceFiles()->clear();
    if(m_model.rowCount() > 1)
        m_model.removeRows(0, m_model.rowCount());
    updateUi();
}

void Widget::on_currentSpeedUnitCB_currentIndexChanged(int index) {
    Q_UNUSED(index)
    updateUi();
}

void Widget::on_sourceFilesAddButton_clicked() {
    trfGen.addFilesAsSource(QFileDialog::getOpenFileNames(this,
                                                          "Выбор файлов-образцов",
                                                          trfGen.getTemplateDir(),
                                                          "*.*"));
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
