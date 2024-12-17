#include "statemachine.h"
#include "logger.h"
#include "sensor.h"
#include "globals.h"
#include "dbmanager.h"
#include "qthread.h"
#include "serial.h"

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
    this->processInfo = processInfo;
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

    controlRelays({
        {"waterFill", 0},  // Stop water filling
        {"heating", 0},    // Stop heating
        {"bypass", 0},     // Disable bypass
        {"pump", 0},       // Stop circulation pump
        {"inPressure", 0}, // Turn off 2 bar pressure
        {"cooling", 0}     // Stop cooling
    });

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

void StateMachine::controlRelays(std::initializer_list<std::pair<const char*, int>> relays)
{
    QStringList dataParts;

    for (const auto& [name, value] : relays) {
        if (Sensor::mapName.contains(name)) {
            Sensor::mapName[name]->value = value; // Update internal sensor state
            dataParts.append(QString("%1=%2;").arg(name).arg(value)); // Append the relay data with a semicolon
        } else {
            Logger::warn(QString("Relay '%1' does not exist").arg(name));
        }
    }

    if (!dataParts.isEmpty()) {
        auto data = dataParts.join("");

        auto& serial = Serial::instance();
        serial.sendData(data); // Send all relay data as a single message
    }

}
void StateMachine::tick()
{
    auto wait3minInHeatingState = 1;//3*60;

    if (isRunning()) {
        values = calculateDrFrRValuesFromSensors(process->getId());
    } else {
        values = calculateStateMachineValues();
    }

    switch (state) {
    case State::READY:
        break;

    case State::STARTING:
        Logger::info("StateMachine: Starting");

        controlRelays({
            {"waterFill", 1},  // Start water filling
            {"heating", 1},    // Start heating
            {"bypass", 1}      // Enable bypass
        });

        state = State::FILLING;
        Logger::info("StateMachine: Filling");
        break;

    case State::FILLING:
        if (values.pressure < 0.5)
            break;

        controlRelays({
            {"waterFill", 0},  // Stop water filling
            {"bypass", 0},     // Disable bypass
            {"pump", 1},       // Start circulation pump
            {"inPressure", 1}  // Set pressure to 2 bars
        });

        Logger::info("StateMachine: Filling - Wait 3 min");
        QThread::msleep(wait3minInHeatingState*1000);

        state = State::HEATING;
        Logger::info("StateMachine: Heating");
        break;

    case State::HEATING:

        if (values.temp > processConfig.maintainTemp + 1) {
            controlRelays({{"heating", 0}}); // Turn off heating
        } else if (values.temp < processConfig.maintainTemp - 1) {
            controlRelays({{"heating", 1}}); // Turn on heating
        }

        if (values.pressure > processConfig.maintainPressure + 0.05) {
            controlRelays({{"inPressure", 0}}); // Turn off 2 bar pressure
        } else if (values.pressure < processConfig.maintainPressure - 0.05) {
            controlRelays({{"inPressure", 1}}); // Turn on 2 bar pressure
        }

        if (processConfig.mode == Mode::TARGETF) {
            if (values.sumFr < processInfo.targetF.toDouble())
                break;
        } else if (processConfig.mode == Mode::TIME) {
            if (values.time < processConfig.targetTime)
                break;
        }

        controlRelays({
            {"heating", 0},      // Turn off heating
            {"inPressure", 0},   // Turn off 2 bar pressure
            {"cooling", 1},      // Start cooling
            {"bypass", 1}        // Enable bypass
        });

        state = State::COOLING;
        Logger::info("StateMachine: Cooling");
        break;

    case State::COOLING:
        if (values.tempK > processConfig.finishTemp)
            break;

        controlRelays({
            {"cooling", 0},   // Turn off cooling
            {"bypass", 0}     // Disable bypass
        });

        state = State::FINISHING;
        Logger::info("StateMachine: Finishing");
        break;

    case State::FINISHING:
        controlRelays({
            {"pump", 0}  // Stop circulation pump
        });

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
