#ifndef PROCESSLOG_H
#define PROCESSLOG_H

#include <QList>
#include <QObject>

struct StateMachineValues {
    uint time;
    double temp;
    double tempK;
    double dTemp;
    double pressure;
    int state;
    double Dr;
    double Fr;
    double r;
    double sumFr;
    double sumr;

    QString toString() {
        return QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11").arg(time).arg(temp).arg(tempK).arg(dTemp).arg(pressure).arg(state).arg(Dr).arg(Fr).arg(r).arg(sumFr).arg(sumr);
    }

    static StateMachineValues parse(QString string) {
        auto splited = string.split(",");
        return {
            .time =     splited[0].toUInt(),
            .temp =     splited[1].toDouble(),
            .tempK =    splited[2].toDouble(),
            .dTemp =    splited[3].toDouble(),
            .pressure = splited[4].toDouble(),
            .state = splited[5].toInt(),
            .Dr =       splited[6].toDouble(),
            .Fr =       splited[7].toDouble(),
            .r =        splited[8].toDouble(),
            .sumFr =    splited[9].toDouble(),
            .sumr =     splited[10].toDouble(),
        };
    }
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
    static QList<ProcessLogInfoRow> getAllProcessLogsOrderedDesc(int processId);

};

#endif // PROCESSLOG_H
