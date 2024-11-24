#include "statemachine.h"
#include "logger.h"
#include "sensor.h"
#include "globals.h"
#include "dbmanager.h"

// Constructor
StateMachine::StateMachine(QObject *parent)
    : QObject(parent), state(READY), timer(nullptr), process(nullptr), processLog(nullptr)
{
    timer = new QTimer(this); // Explicitly initialize timer
    connect(timer, &QTimer::timeout, this, &StateMachine::tick); // Ensure tick is connected
}

// Definition of getState method
int StateMachine::getState() {
    return static_cast<int>(state);
}

double StateMachine::calculateDeltaTemperature(double temp)
{
    switch (processConfig.type) {
    case Type::STERILIZATION:
        return Globals::sterilizationTemp - temp;
    case Type::PASTERIZATION:
        return Globals::pasterizationTemp - temp;
    case Type::CUSTOM:
        return processConfig.customTemp - temp;
    default:
        return Globals::sterilizationTemp - temp;
    }
}

bool StateMachine::start(ProcessConfig processConfig, ProcessInfo processInfo)
{
    if (isRunning())
        return false;

    this->processConfig = processConfig;
    state = State::STARTING;

    processStart = QDateTime::currentDateTime();
    process = new Process(processStart.toString(Qt::ISODate), processInfo, this);
    values = StateMachineValues();

    // Start the timer with the state machine tick interval
    QMetaObject::invokeMethod(timer, "start", Qt::AutoConnection, Q_ARG(int, Globals::stateMachineTick));
    tick();

    return true;
}

bool StateMachine::stop()
{
    if (!isRunning())
        return false;

    // Deactivate all sensors
    Sensor::mapName["waterFill"]->send(0);
    Sensor::mapName["heating"]->send(0);
    Sensor::mapName["bypass"]->send(0);

    state = State::READY;
    values = StateMachineValues();

    return true;
}

inline bool StateMachine::isRunning()
{
    return state != State::READY;
}

StateMachineValues StateMachine::getValues()
{
    return values;
}

StateMachineValues StateMachine::calculateStateMachineValues()
{
    StateMachineValues stateMachineValues;

    auto sensorValues = Sensor::getValues();

    stateMachineValues.temp = sensorValues.temp;
    stateMachineValues.tempK = sensorValues.tempK;
    stateMachineValues.pressure = sensorValues.pressure;

    stateMachineValues.dTemp = calculateDeltaTemperature(stateMachineValues.tempK);

    stateMachineValues.Dr = qPow(10, 0.1 * stateMachineValues.dTemp) * (Globals::stateMachineTick / 60000.0);
    stateMachineValues.Fr = qPow(10, -0.1 * stateMachineValues.dTemp) * (Globals::stateMachineTick / 60000.0);
    stateMachineValues.r = 5 * stateMachineValues.Fr;

    stateMachineValues.state = state;

    if (isRunning()) {
        stateMachineValues.time = processStart.msecsTo(QDateTime::currentDateTime());
        stateMachineValues.sumFr = values.sumFr + stateMachineValues.Fr;
        stateMachineValues.sumr = values.sumr + stateMachineValues.r;
    } else {
        stateMachineValues.time = 0;
        stateMachineValues.sumFr = 0;
        stateMachineValues.sumr = 0;
    }

    return stateMachineValues;
}


StateMachineValues StateMachine::calculateDrFrRValuesFromSensors(int processId)
{
    auto stateMachineValues = calculateStateMachineValues();

    DbManager::instance().createProcessLog(processId);

    return stateMachineValues;
}

StateMachineValues StateMachine::calculateDrFrRValuesFromSensorsOnTheFly()
{
    return calculateStateMachineValues();
}

void StateMachine::tick()
{
    values = calculateDrFrRValuesFromSensors(process->getId());

    switch (state) {
    case State::READY:

        break;

    case State::STARTING:
        Logger::info("StateMachine: Starting");

        Sensor::mapName["waterFill"]->send(1);  // Start water filling
        Sensor::mapName["heating"]->send(1);    // Start heating
        Sensor::mapName["bypass"]->send(1);     // Enable bypass

        state = State::FILLING;
        Logger::info("StateMachine: Filling");
        break;

    case State::FILLING:
        // Wait until pressure reaches 0.5 bar
        if (values.pressure < 0.5)
            break;

        Sensor::mapName["waterFill"]->send(0);  // Stop water filling
        Sensor::mapName["bypass"]->send(0);     // Disable bypass
        Sensor::mapName["pump"]->send(1);       // Start circulation pump
        Sensor::mapName["inPressure"]->send(1); // Set pressure to 2 bars

        state = State::HEATING;
        Logger::info("StateMachine: Heating");
        break;

    case State::HEATING:
        // Maintain the set temperature within a ±1°C range
        if (values.temp > processConfig.maintainTemp + 1)
            Sensor::mapName["heating"]->send(0); // Turn off heating
        else if (values.temp < processConfig.maintainTemp - 1)
            Sensor::mapName["heating"]->send(1); // Turn on heating

        // Maintain the set pressure within a ±0.05 bar range
        if (values.pressure > processConfig.maintainPressure + 0.05)
            Sensor::mapName["inPressure"]->send(0); // Turn off 2 bar pressure
        else if (values.pressure < processConfig.maintainPressure - 0.05)
            Sensor::mapName["inPressure"]->send(1); // Turn on 2 bar pressure

        // Continue heating until the target F value or time is reached
        if (processConfig.mode == Mode::TARGETF) {
            if (values.sumFr < processConfig.targetF)
                break;
        } else if (processConfig.mode == Mode::TIME) {
            if (values.time < processConfig.targetTime)
                break;
        }

        Sensor::mapName["heating"]->send(0);        // Turn off heating
        Sensor::mapName["inPressure"]->send(0);     // Turn off 2 bar pressure
        Sensor::mapName["cooling"]->send(1);        // Start cooling by opening magnetic valve
        Sensor::mapName["bypass"]->send(1);         // Enable bypass

        state = State::COOLING;
        Logger::info("StateMachine: Cooling");
        break;

    case State::COOLING:
        // Cool down until the target temperature is reached
        if (values.tempK > processConfig.finishTemp)
            break;

        Sensor::mapName["cooling"]->send(0);    // Turn off cooling
        Sensor::mapName["bypass"]->send(0);     // Disable bypass

        state = State::FINISHING;
        Logger::info("StateMachine: Finishing");
        break;

    case State::FINISHING:
        Sensor::mapName["pump"]->send(0);   // Stop circulation pump

        state = State::FINISHED;
        Logger::info("StateMachine: Finished");
        break;

    case State::FINISHED:
        timer->stop();

        Logger::info("StateMachine: Ready");

        if (process) {
            auto processInfo = process->getInfo();
            processInfo.processLength = QString::number(values.time);
            process->setInfo(processInfo);
        }

        state = State::READY;
        values = StateMachineValues();

        break;
    }
}

StateMachine &StateMachine::instance()
{
    static StateMachine _instance;
    return _instance;
}
