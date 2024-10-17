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
    double Dr;
    double Fr;
    double r;
    double sumFr;
    double sumr;

    QString toString() {
        return QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10").arg(time).arg(temp).arg(tempK).arg(dTemp).arg(pressure).arg(Dr).arg(Fr).arg(r).arg(sumFr).arg(sumr);
    }

    static StateMachineValues parse(QString string) {
        auto splited = string.split(",");
        return {
            .time =     splited[0].toUInt(),
            .temp =     splited[1].toDouble(),
            .tempK =    splited[2].toDouble(),
            .dTemp =    splited[3].toDouble(),
            .pressure = splited[4].toDouble(),
            .Dr =       splited[5].toDouble(),
            .Fr =       splited[6].toDouble(),
            .r =        splited[7].toDouble(),
            .sumFr =    splited[8].toDouble(),
            .sumr =     splited[9].toDouble(),
        };
    }
};

class ProcessLog : public QObject
{
    Q_OBJECT

    int id = -1;
    QString name;
    QList<StateMachineValues> logs;

public:
    explicit ProcessLog();
    explicit ProcessLog(int id, QString name, QObject *parent = nullptr);

    int getId();
    QString getName();    
    QList<StateMachineValues> getLogs();

private:    
    int saveProcessLog();

};

#endif // PROCESSLOG_H
