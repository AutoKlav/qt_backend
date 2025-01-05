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
        READY, STARTING, FILLING, HEATING, COOLING, COOLING_HELPER, FINISHING, FINISHED
    };

    enum Type {
        STERILIZATION, PASTERIZATION, CUSTOM
    };

    enum Mode {
        TARGETF, TIME
    };

    enum HeatingType {
        STEAM, ELECTRIC
    };

    struct ProcessConfig {
        Type type;
        HeatingType heatingType;
        double customTemp;
        Mode mode;        
        double maintainTemp;
        double finishTemp;
        double d0, z;
    };

    bool start(ProcessConfig processConfig, ProcessInfo processInfo);
    bool stop();
    bool isRunning();
    int getState();

    bool startManualMeasuring();
    bool stopManualMeasuring();

    StateMachineValues getValues();

    StateMachineValues calculateStateMachineValues();
    StateMachineValues calculateDrFrRValuesAndUpdateDbFromSensors(int processId);
    StateMachineValues getStateMachineValuesOnTheFly();
    
    static StateMachine &instance();

private:
    explicit StateMachine(QObject *parent = nullptr);

    QTimer *timer;
    Process *process;
    ProcessLog *processLog;
    State state;
    QDateTime processStart;
    QDateTime heatingStart;
    QDateTime coolingStart;
    QDateTime stopwatch1;
    uint heatingTime;
    uint coolingTime;
    StateMachineValues stateMachineValues;
    ProcessConfig processConfig;
    ProcessInfo processInfo;

    quint64 id;

private slots:
    void tick();
    void autoklavControl();
    void tankControl();
    void pipeControl();
    void verificationControl();
    void manualTick();
};

#endif // STATEMACHINE_H
