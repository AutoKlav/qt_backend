#ifndef PROCESSLOG_H
#define PROCESSLOG_H

#include <QList>
#include <QObject>
#include "sensor.h"

struct StateMachineValues : SensorValues {
    uint time;
    double dTemp;
    double pressure;
    unsigned short state;
    double Dr;
    double Fr;
    double r;
    double sumFr;
    double sumr;   
};
 
struct ProcessLogInfoRow : StateMachineValues {
    int processId;    
    QString timestamp;
};

class ProcessLog : public QObject
{
    Q_OBJECT

    QString name;
    QList<StateMachineValues> logs;

public:
    explicit ProcessLog();
    explicit ProcessLog(QString name, QObject *parent = nullptr);

    QString getName();    
    QList<StateMachineValues> getLogs();
    static QList<ProcessLogInfoRow> getAllProcessLogs(int processId);

};

#endif // PROCESSLOG_H
