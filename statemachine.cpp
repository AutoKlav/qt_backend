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
    connect(timer, &QTimer::timeout, this, &StateMachine::tick); // Ensure tick is connected
}


void StateMachine::tick()
{
    verificationControl();
    //autoklavControl();
    //tankControl();
    //pipeControl();
}

void StateMachine::pipeControl()
{
    Logger::info("### Expansion ###");

    if(values.expansionTemp >= Globals::expansionTemp){
        Sensor::mapName[CONSTANTS::EXTENSION_COOLING]->send(1);
    }
    else {
        Sensor::mapName[CONSTANTS::EXTENSION_COOLING]->send(0);
    }
}

void StateMachine::verificationControl()
{
    Logger::info("### Verification ###");

    if(values.doorClosed == 1 ||
        values.burnerFault == 1 ||
        values.waterShortage == 1 ){
        Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(1);

        // Use QTimer::singleShot to wait for 5 seconds and then turn off the alarm
        QTimer::singleShot(5000, this, [this]() {
            Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(0);
        });
    }
}

void StateMachine::tankControl()
{
    Logger::info("### Tank ###");

    if(values.tankWaterLevel > 40){        
        // mantain tank temp at 95
        if(values.tankTemp > 96){
            Sensor::mapName[CONSTANTS::TANK_HEATING]->send(1);
        } else if(values.tankTemp < 94){
            Sensor::mapName[CONSTANTS::TANK_HEATING]->send(0);
        }

        if(values.tankWaterLevel > 80){
            Sensor::mapName[CONSTANTS::FILL_TANK_WITH_WATER]->send(0);
        }
    }
    else{
        Sensor::mapName[CONSTANTS::TANK_HEATING]->send(0);
    }
}

// Definition of getState method
int StateMachine::getState() {
    return static_cast<int>(state);
}

bool StateMachine::start(ProcessConfig processConfig, ProcessInfo processInfo)
{
    if (isRunning()){
        Logger::info("Autoklav is already running");
        return false;
    }

    // Fetch first time values and abort start if door is not closed
    values = calculateStateMachineValues();

    if (!values.doorClosed){
        Logger::info("Door is not closed");
        return false;
    }

    this->processConfig = processConfig;
    this->processInfo = processInfo;
    state = State::STARTING;

    processStart = QDateTime::currentDateTime();
    process = new Process(processStart.toString(Qt::ISODate), processInfo, this);
    values = StateMachineValues();

    // Start the timer with the state machine tick interval
    QMetaObject::invokeMethod(timer, "start", Qt::AutoConnection, Q_ARG(int, Globals::stateMachineTick));
    autoklavControl();

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

    const auto k = Globals::k;
    const auto exp = stateMachineValues.dTemp / processConfig.z;

    // Old autoklav
    //stateMachineValues.Dr = qPow(10, 0.1 * stateMachineValues.dTemp) * (Globals::stateMachineTick / 60000.0);
    //stateMachineValues.Fr = qPow(10, -0.1 * stateMachineValues.dTemp) * (Globals::stateMachineTick / 60000.0);
    //stateMachineValues.r = 5 * stateMachineValues.Fr;

    // TODO check if correct
    stateMachineValues.Dr = k * processConfig.d0 * qPow(10, -1*exp) * (Globals::stateMachineTick / 60000.0);
    stateMachineValues.Fr = (qPow(10, exp) * (Globals::stateMachineTick / 60000.0)) / (k * processConfig.d0);
    stateMachineValues.r = (qPow(10, exp) * (Globals::stateMachineTick / 60000.0)) / processConfig.d0 ;

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

void StateMachine::autoklavControl()
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
        stopwatch1 = QDateTime::currentDateTime().addSecs(180); // 3 minutes

        state = State::FILLING;
        Logger::info("StateMachine: Filling");
        break;

    case State::FILLING:

        if(QDateTime::currentDateTime() < stopwatch1){
            Logger::info("Wait 10min");
            break;
        }

        Sensor::mapName[CONSTANTS::PUMP]->send(1); // Turn on pump
        Sensor::mapName[CONSTANTS::AUTOKLAV_FILL]->send(0); // Turn off autoklav water fill
        Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(1); // Turn on pressure increase

        if(values.pressure < 0.1 && values.tankWaterLevel < 40){
            break;
        }


        Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(0); // Turn off pressure increase
        Sensor::mapName[CONSTANTS::HEATING]->send(1); // Turn on autoklav heating with steem

        heatingStart =  QDateTime::currentDateTime();

        state = State::HEATING;
        Logger::info("StateMachine: Heating");
        break;

    case State::HEATING:

        if (values.temp > processConfig.maintainTemp + 1) {
            Sensor::mapName[CONSTANTS::HEATING]->send(0); // Turn off heating
        } else if (values.temp < processConfig.maintainTemp - 1) {
            Sensor::mapName[CONSTANTS::HEATING]->send(1); // Turn on heating
        }

        if (processConfig.mode == Mode::TARGETF) {
            if (values.sumFr < processInfo.targetF.toDouble())
                break;
        } else if (processConfig.mode == Mode::TIME) {
            if (heatingStart.msecsTo(QDateTime::currentDateTime()) < processInfo.targetHeatingTime.toDouble())
                break;
        }

        Sensor::mapName[CONSTANTS::PUMP]->send(0);

        heatingTime = heatingStart.msecsTo(QDateTime::currentDateTime());

        // tank container A3, should be mantained at 95

        // 4.2
        Sensor::mapName[CONSTANTS::HEATING]->send(0);        // Turn off heating
        Sensor::mapName[CONSTANTS::COOLING]->send(1);        // Turn on main cooling
        Sensor::mapName[CONSTANTS::COOLING_HELPER]->send(1); // Turn on help cooling

        if(values.tankWaterLevel < 40)
            Sensor::mapName[CONSTANTS::FILL_TANK_WITH_WATER]->send(1);

        coolingStart = QDateTime::currentDateTime();

        state = State::COOLING;

        Logger::info("StateMachine: Cooling");
        break;

    case State::COOLING:

        if (values.tempK > Globals::coolingThreshold)
            break;

        Sensor::mapName[CONSTANTS::COOLING]->send(0);    // Turn off cooling

        if(values.tankWaterLevel < 80)
            break;


        if(values.temp > processConfig.finishTemp && values.tempK > processConfig.finishTemp){
            stopwatch1 = QDateTime::currentDateTime().addSecs(600); // 10 minutes
            coolingTime = coolingStart.msecsTo(QDateTime::currentDateTime());
            break;
        }

        Sensor::mapName[CONSTANTS::COOLING_HELPER]->send(0);

        if(QDateTime::currentDateTime() < stopwatch1){
            Logger::info("Wait 10min");
            break;
        }

        Sensor::mapName[CONSTANTS::WATER_DRAIN]->send(1);
        stopwatch1 = QDateTime::currentDateTime().addSecs(600); // 10 minutes

        state = State::FINISHING;
        Logger::info("StateMachine: Finishing");
        break;

    case State::FINISHING:

        // sleep(10min)
        if(QDateTime::currentDateTime() < stopwatch1){
            break;
        }

        Sensor::mapName[CONSTANTS::WATER_DRAIN]->send(0);
        Sensor::mapName[CONSTANTS::PUMP]->send(0);

        state = State::FINISHED;
        Logger::info("StateMachine: Finished");
        break;

    case State::FINISHED:
        timer->stop();

        Logger::info("StateMachine: Ready");

        if (process) {
            auto processInfo = process->getInfo();
            processInfo.processLength = QString::number(values.time);
            processInfo.targetCoolingTime = QString::number(coolingTime);
            processInfo.targetHeatingTime = QString::number(heatingTime);
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
