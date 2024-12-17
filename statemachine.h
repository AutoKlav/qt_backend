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
        uint64_t targetTime;
        double maintainTemp;
        double maintainPressure;
        double finishTemp;
    };

    bool start(ProcessConfig processConfig, ProcessInfo processInfo);
    bool stop();
    bool isRunning();
    int getState(); 

    StateMachineValues getValues();

    StateMachineValues calculateStateMachineValues();
    StateMachineValues calculateDrFrRValuesFromSensors(int processId);
    StateMachineValues calculateDrFrRValuesFromSensorsOnTheFly();
    
    static StateMachine &instance();

private:
    explicit StateMachine(QObject *parent = nullptr);

    QTimer *timer;
    Process *process;
    ProcessLog *processLog;
    State state;
    QDateTime processStart;
    StateMachineValues values;
    ProcessConfig processConfig;
    ProcessInfo processInfo;

    quint64 id;

    double calculateDeltaTemperature(double temp);

    /**
     * @brief Controls multiple relays at once by specifying relay names and their desired states.
     *
     * This method takes a list of relay name-value pairs, where the name is the identifier
     * of the relay, and the value is the desired state (1 = ON, 0 = OFF). The method ensures
     * that only known relays are controlled.
     *
     * @param relays An initializer list of relay name-state pairs.
     *               Example: {{"waterFill", 1}, {"heating", 0}}
     *
     * @note If the relay name does not exist in Sensor::mapName, a warning is logged.
     * @note This method also logs the change for each relay.
     *
     * **Usage Example:**
     * @code
     * controlRelays({
     *     {"waterFill", 1},
     *     {"heating", 0},
     *     {"bypass", 1}
     * });
     * @endcode
     */
    void controlRelays(std::initializer_list<std::pair<const char*, int>> relays);

private slots:
    void tick();
};

#endif // STATEMACHINE_H
