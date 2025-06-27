//
// Created by hristo on 6/27/25.
//

#pragma once

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <iostream>

class SystemMonitor : public QObject {
    Q_OBJECT
public:
    explicit SystemMonitor(QObject *parent = nullptr)
        : QObject(parent),
          timer(new QTimer(this))
    {
        connect(timer, &QTimer::timeout, this, &SystemMonitor::updateSystemStats);
    }

    void startMonitoring() {
        QFile tempCheck(tempSensorPath);
        if (!tempCheck.exists()) {
            qWarning() << "Температурният сензор не е намерен.";
        }

        //readCpuTimes(); // инициализация

        timer->start(1000); // 1 секунда
    }

    void stopMonitoring() {
        if (timer->isActive())
            timer->stop();
    }

signals:
    void cpuUsageUpdated(double usagePercent);
    void cpuTemperatureUpdated(double temperatureCelsius);

private:
    QTimer *timer;
    QString tempSensorPath = "/sys/class/thermal/thermal_zone0/temp";

    // CPU usage tracking
    quint64 prevTotal = 0;
    quint64 prevWork = 0;

    void updateSystemStats() {
        updateCpuUsage();
        updateCpuTemperature();
    }

    void updateCpuUsage() {
        QFile file("/proc/stat");
        if (file.open(QIODevice::ReadOnly)) {
            QString line = file.readLine();
            if (line.startsWith("cpu ")) {
                QTextStream ts(&line);
                QString cpu;
                quint64 user, nice, system, idle, iowait, irq, softirq, steal;

                ts >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

                quint64 workTime = user + nice + system + irq + softirq + steal;
                quint64 totalTime = workTime + idle + iowait;

                if (prevTotal > 0) {
                    quint64 totalDiff = totalTime - prevTotal;
                    quint64 workDiff = workTime - prevWork;

                    if (totalDiff > 0) {
                        double usage = (workDiff * 100.0) / totalDiff;
                        double smooth = exponentialSmoothing(usage);
                        emit cpuUsageUpdated(smooth);
                        ///std::cout << "CPU Usage: " << smooth << " %" << std::endl;
                    }
                }

                prevTotal = totalTime;
                prevWork = workTime;
            }
        }
    }

    void updateCpuTemperature() {
        QFile file(tempSensorPath);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream ts(&file);
            int tempMilliC = 0;
            ts >> tempMilliC;

            double tempC = tempMilliC / 1000.0;
            emit cpuTemperatureUpdated(tempC);
            ///std::cout << "CPU Temperature: " << tempC << " °C" << std::endl;
        }
    }

    double exponentialSmoothing(double newValue) {
        static double smoothedValue = 0.0;
        const double alpha = 0.3;

        if (smoothedValue == 0.0)
            return smoothedValue = newValue;

        return smoothedValue = alpha * newValue + (1.0 - alpha) * smoothedValue;
    }

    void readCpuTimes() {
        QFile file("/proc/stat");
        if (file.open(QIODevice::ReadOnly)) {
            QString line = file.readLine();
            if (line.startsWith("cpu ")) {
                QTextStream ts(&line);
                QString cpu;
                quint64 user, nice, system, idle, iowait, irq, softirq, steal;

                ts >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

                prevWork = user + nice + system + irq + softirq + steal;
                prevTotal = prevWork + idle + iowait;
            }
        }
    }
};

