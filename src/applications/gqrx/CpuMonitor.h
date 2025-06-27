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
        readCpuTimes();
        timer->start(1000);
    }

private:
    QTimer *timer;
    quint64 prevTotal = 0;
    quint64 prevWork = 0;

    void readCpuTimes() {
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
                        double cpuUsage = (workDiff * 100.0) / totalDiff;
                        double smooth = exponentialSmoothing(cpuUsage);
                        std::cout << "CPU usage: " << smooth << "%" << std::endl;
                    }
                }

                prevTotal = totalTime;
                prevWork = workTime;
            }
        }
    }

    double exponentialSmoothing(double newValue) {
        static double smoothedValue = 0.0;
        const double alpha = 0.3;
        
        if (smoothedValue == 0.0) {
            return smoothedValue = newValue;
        }
        return smoothedValue = alpha * newValue + (1.0 - alpha) * smoothedValue;
    }

private slots:
    void updateCpuUsage() {
        readCpuTimes();
    }
};