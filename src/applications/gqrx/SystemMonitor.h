#pragma once

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QDebug>

class SystemMonitor : public QObject {
    Q_OBJECT
public:
    explicit SystemMonitor(QObject *parent = nullptr);
    ~SystemMonitor() override;

    void startMonitoring();
    void stopMonitoring();

signals:
    void cpuUsageUpdated(double usagePercent);
    void cpuTemperatureUpdated(double temperatureCelsius);

private:
    struct CpuStats {
        quint64 workTime;
        quint64 totalTime;
    };

    static constexpr double SMOOTHING_FACTOR = 0.3;
    static constexpr const char* PROC_STAT_PATH = "/proc/stat";
    
    QTimer* timer;
    QString tempSensorPath;
    
    CpuStats previousStats{0, 0};
    double smoothedValue{0.0};

    void updateSystemStats();
    void updateCpuUsage();
    void updateCpuTemperature();
    double exponentialSmoothing(double newValue);
    CpuStats readCpuTimes();
    CpuStats parseCpuStats(const QString& line);
};