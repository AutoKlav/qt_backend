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
        READY, STARTING, FILLING, HEATING, STERILIZING, PRECOOLING,  COOLING, FINISHING, FINISHED
    };

    enum Mode {
        TARGETF, TIME
    };

    enum HeatingType {
        STEAM, ELECTRIC, STEAM_ELECTRIC
    };

    struct ProcessConfig {        
        HeatingType heatingType;        
        Mode mode;
    };

    bool start(ProcessConfig processConfig, ProcessInfo processInfo);
    bool stop();
    bool isRunning();
    int getState();    

    StateMachineValues getValues();
    StateMachineValues readInputPinValues();
    StateMachineValues calculateStateMachineValues();

    QString getHeatingEnd() const { return heatingEnd; }
    QString getCoolingEnd() const { return coolingEnd; }

    
    static StateMachine &instance();

private:
    explicit StateMachine(QObject *parent = nullptr);

    QTimer timer;
    Process *process;
    State state;
    QDateTime processStart;
    QDateTime heatingStart;
    QString heatingEnd;
    QDateTime coolingStart;
    QString coolingEnd;
    QDateTime stopwatch1;
    QDateTime writeInDBstopwatch;
    StateMachineValues stateMachineValues;
    ProcessConfig processConfig;
    ProcessInfo processInfo;

    quint64 id;

    bool verificationControl();
    void triggerAlarm();

private slots:
    void tick();
    void autoklavControl();
    void tankControl();
    void pipeControl();
};

#endif // STATEMACHINE_H
