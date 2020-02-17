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

#define     VERSION               "v1.2.25:3"

#define     DEFAULT_GENERATE_INTERVAL           3
#define     DEFAULT_FILES_NB_PER_INTERVAL       1
#define     DEFAULT_INFECTED_FILE_PROBABILITY   0.2
#define     DEFAULT_UNIT                        5

enum FILE_TYPE {
    CLEAN,
    INFECTED
};

const static QDir::Filters usingFilters = QDir::Files | QDir::NoSymLinks;

class TrafficGenerator : public QObject {

    Q_OBJECT

    bool m_workStatus{false};

    QFileInfoList m_cleanFiles;
    QString m_cleanFilesDir;

    QFileInfoList m_infectedFiles;
    QString m_infectedFilesDir;

    QString m_destinationDir;

    int m_generateInterval{DEFAULT_GENERATE_INTERVAL};
    int m_filesPerInterval{DEFAULT_FILES_NB_PER_INTERVAL};

    double m_totalVolInBytes{NULL};
    double m_currentSpeedInBytes{NULL};
    double m_averageSpeedInBytes{NULL};
    int m_globalCnt{NULL};
    int m_infectedFilesCnt{NULL};

    double m_infectedFileGenerateProbability{DEFAULT_INFECTED_FILE_PROBABILITY};

    QDateTime m_startTime;
    QDateTime m_endTime;

public:
    TrafficGenerator();

    void setWorkStatus(bool workStatus);
    bool getWorkStatus() const;

    void setCleanFilesDir(const QString& cleanFilesDir);
    QString getCleanFilesDir() const;

    void setCleanFiles(const QFileInfoList& cleanFiles);
    QFileInfoList& getCleanFiles();

    void setInfectedFiles(const QFileInfoList& infectedFiles);
    QFileInfoList& getInfectedFiles();

    QString getInfectedFilesDir() const;
    void setInfectedFilesDir(const QString& infectedFilesDir);

    void setDestinationDir(const QString& destinationDir);
    QString getDestinationDir() const;

    void setGenerateInterval(int generateInterval);
    int getGenerateInterval() const;

    void setFilesPerInterval(int filesPerInterval);
    int getFilesPerInterval() const;

    double getInfectedFileGenerateProbability() const;
    void setInfectedFileGenerateProbability(double infectedFileGenerateProbability);

    void addCleanFiles(QStringList fileNames);
    void addInfectedFiles(QStringList fileNames);

// statistics
    double getCurrentSpeedInBytes() const;
    double getAverageSpeedInBytes() const;
    double getTotalVolInBytes() const;
    int getGlobalCnt() const;
    int getInfectedFilesNb() const;
    void flushStatistic();

    qint64 getWorkTimeInSecs();
    QDateTime getStartTime() const;
    QDateTime getEndTime() const;

// core
    void start();
    void generate();
    void generateFile(QFileInfoList &sourceList);
    void stop();

signals:
    void updateProgressBar(int newVal);
    void executeError(int code);
    void updateUi();
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

private:
    Ui::Widget *ui;
    QSettings settings;

    TrafficGenerator trfGen;
    QThread trafficThread;

    QDateTime m_startDateTime;
    QDateTime m_endDateTime;

    QStandardItemModel m_cleanFilesModel;
    QStandardItemModel m_infectedFilesModel;

    QTimer m_updateTimer;
    QTimer m_generateTimer;

public:
    void updateUi();
    void prepareTables();
    void setGenerationInterval(int secs);
    void executeError(int code);

private slots:
    void on_generationFileNbSB_valueChanged(int newVal);
    void on_generationIntervalSB_valueChanged(int newVal);
    void on_destinationDirButton_clicked();
    void on_infectedFileProbabilityDSB_valueChanged(double probability);

    void on_startButton_clicked();
    void on_stopButton_clicked();

    void on_currentSpeedUnitCB_currentIndexChanged(int index);
    void on_averageSpeedUnitCB_currentIndexChanged(int index);
    void on_totalVolumeUnitCB_currentIndexChanged(int index);

    void on_cleanFilesAddButton_clicked();
    void on_cleanFilesClearButton_clicked();

    void on_infectedFilesAddButton_clicked();
    void on_infectedFilesClearButton_clicked();
};

double convert(double bytes, int outType);

#endif // WIDGET_H

