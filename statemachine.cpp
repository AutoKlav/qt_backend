#include "statemachine.h"
#include "logger.h"
#include "sensor.h"
#include "globals.h"
#include "dbmanager.h"
#include "constants.h"

// Constructor
StateMachine::StateMachine(QObject *parent)
    : QObject(parent), state(READY), timer(nullptr), process(nullptr), processLog(nullptr)
{
    timer = new QTimer(this); // Explicitly initialize timer
    connect(timer, &QTimer::timeout, this, &StateMachine::autoklavTickControl); // Ensure tick is connected
}

// Definition of getState method
int StateMachine::getState() {
    return static_cast<int>(state);
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
    autoklavTickControl();
    //tankTickControl();
    //pipeTickControl();

    return true;
}

bool StateMachine::stop()
{
    if (!isRunning())
        return false;

    Sensor::mapName[CONSTANTS::FILL_TANK_WITH_WATER]->send(0);
    Sensor::mapName[CONSTANTS::COOLING]->send(0);
    Sensor::mapName[CONSTANTS::TANK_HEATING]->send(0);
    Sensor::mapName[CONSTANTS::COOLING_HELPER]->send(0);
    Sensor::mapName[CONSTANTS::AUTOKLAV_FILL]->send(0);
    Sensor::mapName[CONSTANTS::WATER_DRAIN]->send(0);
    Sensor::mapName[CONSTANTS::HEATING]->send(0);
    Sensor::mapName[CONSTANTS::PUMP]->send(0);
    Sensor::mapName[CONSTANTS::ELECTRIC_HEATING]->send(0);
    Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(0);
    Sensor::mapName[CONSTANTS::EXTENSION_COOLING]->send(0);
    Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(0);

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

StateMachineValues StateMachine::getStateMachineValuesOnTheFly()
{
    return calculateStateMachineValues();
}

StateMachineValues StateMachine::calculateStateMachineValues()
{
    StateMachineValues stateMachineValues;

    auto sensorValues = Sensor::getValues();

    stateMachineValues.temp = sensorValues.temp;
    stateMachineValues.expansionTemp = sensorValues.expansionTemp;
    stateMachineValues.heaterTemp = sensorValues.heaterTemp;
    stateMachineValues.tankTemp = sensorValues.tankTemp;
    stateMachineValues.tempK = sensorValues.tempK;
    stateMachineValues.tankWaterLevel = sensorValues.tankWaterLevel;
    stateMachineValues.pressure = sensorValues.pressure;
    stateMachineValues.steamPressure = sensorValues.steamPressure;

    stateMachineValues.doorClosed = sensorValues.doorClosed;
    stateMachineValues.burnerFault = sensorValues.burnerFault;
    stateMachineValues.waterShortage = sensorValues.waterShortage;

    stateMachineValues.dTemp = processConfig.customTemp - stateMachineValues.tempK;

    // TODO import from globals
    const auto k = 1;
    const auto exp = (Globals::stateMachineTick / 60000.0) / processConfig.z;

    stateMachineValues.Dr = k * processConfig.d0 * qPow(10, -1*exp);
    stateMachineValues.Fr = qPow(10, exp) / (k * processConfig.d0) ;
    stateMachineValues.r = qPow(10, exp) / processConfig.d0;

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

void StateMachine::autoklavTickControl()
{    
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

        Sensor::mapName[CONSTANTS::AUTOKLAV_FILL]->send(1); // Turn on autoklav water fill
        Sensor::mapName[CONSTANTS::HEATING]->send(1);    // Start heating        

        state = State::FILLING;
        Logger::info("StateMachine: Filling");
        break;

    case State::FILLING:
        if (values.time > 3000) // After 3 min
            Sensor::mapName[CONSTANTS::PUMP]->send(1); // Turn on pump

        Sensor::mapName[CONSTANTS::AUTOKLAV_FILL]->send(0); // Turn off autoklav water fill
        Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(1); // Turn on pressure increase

        state = State::PRESSURING;
        Logger::info("StateMachine: Pressuing");
        break;

    case State::PRESSURING:
        if (values.pressure < 1.0) // Wait till autoklav pressure is 1bar
            break;

        Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(0); // Turn off pressure increase
        Sensor::mapName[CONSTANTS::HEATING]->send(1); // Turn on autoklav heating with steem

        // TODO: Log heating start so we can later save length of heating
        // auto heatingStarted = datetime.now();

        Sensor::mapName[CONSTANTS::FILL_TANK_WITH_WATER]->send(0); // Stop above tank water filling
        Sensor::mapName[CONSTANTS::COOLING]->send(0);
        Sensor::mapName[CONSTANTS::PUMP]->send(1);       // Start circulation pump
        Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(1);       // Start pressure       

        state = State::HEATING;
        Logger::info("StateMachine: Heating");
        break;

    case State::HEATING:

        if (values.temp > processConfig.maintainTemp + 1) {
            Sensor::mapName[CONSTANTS::HEATING]->send(0); // Turn off heating
        } else if (values.temp < processConfig.maintainTemp - 1) {
            Sensor::mapName[CONSTANTS::HEATING]->send(1); // Turn on heating
        }

        if (values.pressure > processConfig.maintainPressure + 0.05) {
            Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(0); // Turn off 2 bar pressure
        } else if (values.pressure < processConfig.maintainPressure - 0.05) {
            Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(1); // Turn on 2 bar pressure
        }

        if (processConfig.mode == Mode::TARGETF) {
            if (values.sumFr < processInfo.targetF.toDouble())
                break;
        } else if (processConfig.mode == Mode::TIME) {
            // if (datetime.now() - heatingStarted < processConfig.targetHeatingTime)
            // TODO: Calcualte time of heating, not whole proccess time
        }

        Sensor::mapName[CONSTANTS::HEATING]->send(0);        // Turn off heating
        Sensor::mapName[CONSTANTS::COOLING]->send(1);        // Turn on main cooling
        Sensor::mapName[CONSTANTS::COOLING_HELPER]->send(1); // Turn on help cooling

        // process.heatingTime = datetime.now() - heatingStarted;

        state = State::COOLING;
        Logger::info("StateMachine: Cooling");
        break;

    case State::COOLING:
        if (values.tempK > processConfig.finishTemp)
            break;

        Sensor::mapName[CONSTANTS::COOLING]->send(0);    // Turn off cooling
        //Sensor::mapName["bypass"]->send(0);

        state = State::FINISHING;
        Logger::info("StateMachine: Finishing");
        break;

    case State::FINISHING:
        Sensor::mapName[CONSTANTS::PUMP]->send(0);   // Stop circulation pump

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
