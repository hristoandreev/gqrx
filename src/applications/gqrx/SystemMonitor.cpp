
#include "SystemMonitor.h"

SystemMonitor::SystemMonitor(QObject *parent)
    : QObject(parent)
    , timer(new QTimer(this))
    , tempSensorPath("/sys/class/thermal/thermal_zone0/temp")
{
    connect(timer, &QTimer::timeout, this, &SystemMonitor::updateSystemStats);
}

SystemMonitor::~SystemMonitor() = default;

void SystemMonitor::startMonitoring() {
    QFile tempCheck(tempSensorPath);
    if (!tempCheck.exists()) {
        qWarning() << "Temperature sensor not found!";
    }

    previousStats = readCpuTimes();
    timer->start(1000);
}

void SystemMonitor::stopMonitoring() {
    timer->stop();
}

void SystemMonitor::updateSystemStats() {
    updateCpuUsage();
    updateCpuTemperature();
}

void SystemMonitor::updateCpuUsage() {
    QFile file(PROC_STAT_PATH);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QString line = file.readLine();
    if (!line.startsWith("cpu ")) {
        return;
    }

    CpuStats currentStats = parseCpuStats(line);
    
    if (previousStats.totalTime > 0) {
        quint64 totalDiff = currentStats.totalTime - previousStats.totalTime;
        quint64 workDiff = currentStats.workTime - previousStats.workTime;

        if (totalDiff > 0) {
            double usage = (workDiff * 100.0) / totalDiff;
            emit cpuUsageUpdated(exponentialSmoothing(usage));
        }
    }

    previousStats = currentStats;
}

void SystemMonitor::updateCpuTemperature() {
    QFile file(tempSensorPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextStream ts(&file);
    int tempMilliC = 0;
    ts >> tempMilliC;

    double tempC = tempMilliC / 1000.0;
    emit cpuTemperatureUpdated(tempC);
}

double SystemMonitor::exponentialSmoothing(double newValue) {
    if (smoothedValue == 0.0) {
        return smoothedValue = newValue;
    }

    return smoothedValue = SMOOTHING_FACTOR * newValue + (1.0 - SMOOTHING_FACTOR) * smoothedValue;
}

SystemMonitor::CpuStats SystemMonitor::readCpuTimes() {
    QFile file(PROC_STAT_PATH);
    if (!file.open(QIODevice::ReadOnly)) {
        return {0, 0};
    }

    QString line = file.readLine();
    if (!line.startsWith("cpu ")) {
        return {0, 0};
    }

    return parseCpuStats(line);
}

SystemMonitor::CpuStats SystemMonitor::parseCpuStats(const QString& line) {
    QTextStream ts(const_cast<QString*>(&line));
    QString cpu;
    quint64 user, nice, system, idle, iowait, irq, softirq, steal;

    ts >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    quint64 workTime = user + nice + system + irq + softirq + steal;
    quint64 totalTime = workTime + idle + iowait;

    return {workTime, totalTime};
}
