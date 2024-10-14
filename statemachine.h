#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <QTimer>
#include <QObject>
#include <QDateTime>

#include "processlog.h"


class StateMachine : public QObject
{
    Q_OBJECT
public:
    StateMachine(const StateMachine&) = delete;
    StateMachine& operator=(const StateMachine &) = delete;
    StateMachine(StateMachine &&) = delete;
    StateMachine & operator=(StateMachine &&) = delete;
    ~StateMachine() = default;

    enum State {
        READY, STARTING, FILLING, HEATING, COOLING, FINISHING, FINISHED
    };

    enum Type {
        STERILIZATION, PASTERIZATION, CUSTOM
    };

    enum Mode {
        TARGETF, TIME
    };

    struct ProcessConfig {
        Type type;
        double customTemp;
        Mode mode;
        double targetF;
        uint targetTime;
        double maintainTemp;
        double maintainPressure;
        double finishTemp;
    };

    bool start(ProcessConfig processConfig, ProcessInfo processInfo);
    bool stop();
    bool isRunning();

    StateMachineValues getValues();
    StateMachineValues calcValues();

    static StateMachine &instance();

private:
    explicit StateMachine(QObject *parent = nullptr);

    QTimer *timer;
    ProcessLog *processLog;

    State state;
    QDateTime processStart;
    StateMachineValues values;
    ProcessConfig processConfig;

    quint64 id;

    double calcdTemp(double temp);

private slots:
    void tick();
};

#endif // STATEMACHINE_H
