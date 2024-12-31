#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <QTimer>
#include <QObject>
#include <QDateTime>

#include "process.h"
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
        READY, STARTING, FILLING, PRESSURING, HEATING, COOLING, FINISHING, FINISHED
    };

    enum Type {
        STERILIZATION, PASTERIZATION, CUSTOM
    };

    enum Mode {
        TARGETF, TIME
    };

    struct ProcessConfig {
        Type type;
        double customTemp; // do we need delta temp
        Mode mode;
        uint64_t targetTime;
        double maintainTemp;
        double maintainPressure; // not used
        double finishTemp;
        double d0, z;
    };

    bool start(ProcessConfig processConfig, ProcessInfo processInfo);
    bool stop();
    bool isRunning();
    int getState(); 

    StateMachineValues getValues();

    StateMachineValues calculateStateMachineValues();
    StateMachineValues calculateDrFrRValuesFromSensors(int processId);
    StateMachineValues getStateMachineValuesOnTheFly();
    
    static StateMachine &instance();

private:
    explicit StateMachine(QObject *parent = nullptr);

    QTimer *autoklavTimer;
    QTimer *tankTimer;
    Process *process;
    ProcessLog *processLog;
    State state;
    QDateTime processStart;
    StateMachineValues values;
    ProcessConfig processConfig;
    ProcessInfo processInfo;

    quint64 id;

private slots:
    void autoklavTickControl();
    void tankTickControl();
};

#endif // STATEMACHINE_H
