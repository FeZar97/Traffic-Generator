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
#include <QDebug>

const static QDir::Filters usingFilters = QDir::Files | QDir::NoSymLinks;

class TrafficGenerator : public QObject {

    Q_OBJECT

    QWidget* m_parent;

    bool m_workStatus{false};

    QFileInfoList m_sourceFiles;
    QString m_destinationDir{QDir::tempPath()};
    int m_generateInterval{3};
    int m_filesPerInterval{1};
    double m_totalVolInBytes{0};
    double m_currentSpeedInBytes{0};
    double m_averageSpeedInBytes{0};
    int m_globalCnt{0};

    QDateTime m_startTime;
    QDateTime m_endTime;

public:
    TrafficGenerator(QWidget *parent = nullptr);

    void setWorkStatus(bool workStatus);
    bool getWorkStatus() const;

    void addFilesAsSource(QStringList fileNames);
    QFileInfoList* sourceFiles();

    QString destinationDir() const;
    void setDestinationDir(const QString& destinationDir);

    int generateInterval() const;
    void setGenerateInterval(int generateInterval);

    int filesPerInterval() const;
    void setFilesPerInterval(int filesPerInterval);

// statistics
    double currentSpeedInBytes() const;
    double averageSpeedInBytes() const;
    double totalVolInBytes() const;
    int globalCnt() const;

    qint64 getWorkTimeInSecs();

// core
    void generate();

signals:
    void updateProgressBar(int newVal);
    void timerStateChange();
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

    void updateUi();
    void setGenerationInterval(int secs);
    void genTimerStateChange();

private:
    Ui::Widget *ui;
    QSettings settings;
    TrafficGenerator trfGen;
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

