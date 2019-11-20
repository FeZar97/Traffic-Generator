#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDir>
#include <QFile>
#include <QThread>
#include <QTimer>
#include <QRandomGenerator>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QDateTime>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>

const static QDir::Filters usingFilters = QDir::Files | QDir::NoSymLinks;

class TrafficGenerator : public QObject {

    Q_OBJECT

    bool m_workStatus{false};

    QFileInfoList m_sourceFiles;
    QString m_destinationDir;
    QString m_templateDir;
    int m_generateInterval{3};
    int m_filesPerInterval{1};
    double m_totalVolInBytes{0};
    double m_currentSpeedInBytes{0};
    double m_averageSpeedInBytes{0};
    int m_globalCnt{0};

    QDateTime m_startTime;
    QDateTime m_endTime;

public:
    TrafficGenerator();

    void setWorkStatus(bool workStatus);
    bool getWorkStatus() const;

    void addFilesAsSource(QStringList fileNames);
    QFileInfoList* sourceFiles();

    QString destinationDir() const;
    void setDestinationDir(const QString& destinationDir);

    QString getTemplateDir() const;
    void setTemplateDir(const QString& templateDir);

    int generateInterval() const;
    void setGenerateInterval(int generateInterval);

    int filesPerInterval() const;
    void setFilesPerInterval(int filesPerInterval);

// statistics
    double currentSpeedInBytes() const;
    double averageSpeedInBytes() const;
    double totalVolInBytes() const;
    int globalCnt() const;
    void flushStatistic();

    qint64 getWorkTimeInSecs();
    QDateTime getStartTime() const;
    QDateTime getEndTime() const;

// core
    void start();
    void generate();
    void stop();

signals:
    void updateProgressBar(int newVal);
    void executeError(int code);
    void updateUi();
    void setTemplateDir();
};

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void updateUi();
    void setGenerationInterval(int secs);
    void executeError(int code);

private:
    Ui::Widget *ui;
    QSettings settings;
    TrafficGenerator trfGen;
    QDateTime m_startDateTime;
    QDateTime m_endDateTime;
    QThread trafficThread;
    QStandardItemModel m_model;
    QTimer m_updateTimer;
    QTimer m_generateTimer;

private slots:
    void on_generationFileNbSB_valueChanged(int newVal);
    void on_generationIntervalSB_valueChanged(int newVal);
    void on_destinationDirButton_clicked();
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_sourceFilesClearButton_clicked();
    void on_currentSpeedUnitCB_currentIndexChanged(int index);
    void on_sourceFilesAddButton_clicked();
    void on_averageSpeedUnitCB_currentIndexChanged(int index);
    void on_totalVolumeUnitCB_currentIndexChanged(int index);
};

double convert(double bytes, int outType);

#endif // WIDGET_H

