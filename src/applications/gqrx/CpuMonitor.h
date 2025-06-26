//
// Created by wolfberry on 20/06/2025.
//

#pragma once

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <iostream>

class CpuMonitor : public QObject {
    Q_OBJECT
public:
    CpuMonitor(QObject *parent = nullptr) : QObject(parent) {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &CpuMonitor::updateCpuUsage);
    }

    void startMonitoring() {
        readCpuTimes(prevIdle, prevTotal);
        timer->start(1000);  // всяка секунда
    }

private:
    QTimer *timer;
    quint64 prevIdle = 0, prevTotal = 0;

    void readCpuTimes(quint64 &idle, quint64 &total) {
        QFile file("/proc/stat");
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray line = file.readLine();
            QTextStream ts(line);
            QString cpu;
            quint64 user, nice, system, idleTime, iowait, irq, softirq, steal;
            ts >> cpu >> user >> nice >> system >> idleTime >> iowait >> irq >> softirq >> steal;

            idle = idleTime + iowait;
            total = user + nice + system + idle + irq + softirq + steal;
        }
    }

double exponentialSmoothing(double newValue) {
    static double smoothedValue = 0.0;
    const double alpha = 0.2;
    
    if (smoothedValue == 0.0) {
        return smoothedValue = newValue;
    }
    return smoothedValue = alpha * newValue + (1.0 - alpha) * smoothedValue;
}

private slots:
    void updateCpuUsage() {
        quint64 idle, total;
        readCpuTimes(idle, total);

        quint64 deltaIdle = idle - prevIdle;
        quint64 deltaTotal = total - prevTotal;

        if (deltaTotal > 0) {
            double usage = 100.0 * (1.0 - (double)deltaIdle / deltaTotal);
            double smooth = exponentialSmoothing(usage);
            std::cout << "CPU usage: " << smooth << "%" << std::endl;
        }

        prevIdle = idle;
        prevTotal = total;
    }
};
