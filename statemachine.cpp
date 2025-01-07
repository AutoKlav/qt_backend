#include "statemachine.h"
#include "logger.h"
#include "sensor.h"
#include "globals.h"
#include "dbmanager.h"
#include "constants.h"
#include <QRandomGenerator>

// Constructor
StateMachine::StateMachine(QObject *parent)
    : QObject(parent), state(READY), timer(nullptr), process(nullptr), processLog(nullptr)
{
    timer = new QTimer(this); // Explicitly initialize timer
    connect(timer, &QTimer::timeout, this, &StateMachine::autoklavControl); // Ensure tick is connected    
}

void StateMachine::pipeControl()
{
    Logger::info("### Extension ###");

    if(stateMachineValues.expansionTemp >= Globals::expansionTemp){
        Sensor::mapName[CONSTANTS::EXTENSION_COOLING]->send(1);
    }
    else {
        Sensor::mapName[CONSTANTS::EXTENSION_COOLING]->send(0);
    }
}

void StateMachine::triggerAlarm(){
    Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(1);
    Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(0);
    //Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(1);
    //Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(0);
}

void StateMachine::verificationControl()
{
    Logger::info("### Verification [Door, Burner, Water] ###");

    // we control door as long as watertrain is not turned on
    if(stateMachineValues.doorClosed != 1 && Sensor::getRelayValues().waterDrain == 0 ){
        triggerAlarm();
    }

    if(stateMachineValues.burnerFault != 1 ||
        stateMachineValues.waterShortage != 1 ){
        triggerAlarm();
    }
}

void StateMachine::tankControl()
{
    Logger::info("### Tank ###");

    if(stateMachineValues.tankWaterLevel > 40){        
        // mantain tank temp at 95
        if(stateMachineValues.tankTemp > 96){
            Sensor::mapName[CONSTANTS::TANK_HEATING]->send(0);
        } else if(stateMachineValues.tankTemp < 94){
            Sensor::mapName[CONSTANTS::TANK_HEATING]->send(1);
        }

        if(stateMachineValues.tankWaterLevel > 80){
            Sensor::mapName[CONSTANTS::FILL_TANK_WITH_WATER]->send(0);
        }
    }
    else{
        Sensor::mapName[CONSTANTS::TANK_HEATING]->send(0);
    }
}

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
    stateMachineValues = calculateStateMachineValues();

    if (!stateMachineValues.doorClosed) {
        Logger::info("Door is not closed");
        return false;
    }

    if (!stateMachineValues.burnerFault) {
        Logger::info("Burner error");
        triggerAlarm();
        return false;
    }

    if (!stateMachineValues.waterShortage) {
        Logger::info("Water shortage error");
        triggerAlarm();
        return false;
    }

    this->processConfig = processConfig;
    this->processInfo = processInfo;
    state = State::STARTING;

    processStart = QDateTime::currentDateTime();
    process = new Process(processStart.toString(Qt::ISODate), processInfo, this);
    stateMachineValues = StateMachineValues();

    // Start the timer with the state machine tick interval
    QMetaObject::invokeMethod(timer, "start", Qt::AutoConnection, Q_ARG(int, Globals::stateMachineTick));
    autoklavControl();

    return true;
}

bool StateMachine::stop()
{
    if (!isRunning())
        return false;

    Logger::info("End process");
    timer->stop();

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
    stateMachineValues = StateMachineValues();

    return true;
}

bool StateMachine::testRelays()
{
    if (isRunning())
        return false;

    Sensor::mapName[CONSTANTS::AUTOKLAV_FILL]->send(1);

    Sensor::mapName[CONSTANTS::PUMP]->send(1);
    Sensor::mapName[CONSTANTS::HEATING]->send(1);
    Sensor::mapName[CONSTANTS::ELECTRIC_HEATING]->send(1);

    Sensor::mapName[CONSTANTS::AUTOKLAV_FILL]->send(0);
    Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(1);
    Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(0);

    Sensor::mapName[CONSTANTS::ELECTRIC_HEATING]->send(0);
    Sensor::mapName[CONSTANTS::HEATING]->send(0);

    Sensor::mapName[CONSTANTS::COOLING]->send(1);
    Sensor::mapName[CONSTANTS::COOLING_HELPER]->send(1);

    Sensor::mapName[CONSTANTS::FILL_TANK_WITH_WATER]->send(1);
    Sensor::mapName[CONSTANTS::FILL_TANK_WITH_WATER]->send(0);

    Sensor::mapName[CONSTANTS::COOLING]->send(0);
    Sensor::mapName[CONSTANTS::PUMP]->send(0);
    Sensor::mapName[CONSTANTS::COOLING_HELPER]->send(0);

    Sensor::mapName[CONSTANTS::WATER_DRAIN]->send(1);
    Sensor::mapName[CONSTANTS::WATER_DRAIN]->send(0);

    Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(1);
    Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(0);

    Sensor::mapName[CONSTANTS::TANK_HEATING]->send(1);
    Sensor::mapName[CONSTANTS::TANK_HEATING]->send(0);

    Sensor::mapName[CONSTANTS::EXTENSION_COOLING]->send(1);
    Sensor::mapName[CONSTANTS::EXTENSION_COOLING]->send(0);

    return true;
}

inline bool StateMachine::isRunning()
{
    return state != State::READY;
}

StateMachineValues StateMachine::getValues()
{
    return stateMachineValues;
}

StateMachineValues StateMachine::getStateMachineValuesOnTheFly()
{
    return calculateStateMachineValues();
}

StateMachineValues StateMachine::calculateStateMachineValues()
{
    StateMachineValues updateStateMachineValues;

    auto sensorValues = Sensor::getValues();

    updateStateMachineValues.temp = sensorValues.temp;
    updateStateMachineValues.expansionTemp = sensorValues.expansionTemp;
    updateStateMachineValues.heaterTemp = sensorValues.heaterTemp;
    updateStateMachineValues.tankTemp = sensorValues.tankTemp;
    updateStateMachineValues.tempK = sensorValues.tempK;
    updateStateMachineValues.tankWaterLevel = sensorValues.tankWaterLevel;
    updateStateMachineValues.pressure = sensorValues.pressure;
    updateStateMachineValues.steamPressure = sensorValues.steamPressure;

    updateStateMachineValues.doorClosed = sensorValues.doorClosed;
    updateStateMachineValues.burnerFault = sensorValues.burnerFault;
    updateStateMachineValues.waterShortage = sensorValues.waterShortage;

    updateStateMachineValues.dTemp = processConfig.customTemp - updateStateMachineValues.tempK;

    const auto k = Globals::k;
    const auto z = processInfo.bacteria.z;
    const auto d0 = processInfo.bacteria.d0;
    Logger::info(QString::number(k));
    Logger::info(QString::number(z));
    Logger::info(QString::number(d0));

    const auto exp = updateStateMachineValues.dTemp / z;

    // Old autoklav
    //stateMachineValues.Dr = qPow(10, 0.1 * stateMachineValues.dTemp) * (Globals::stateMachineTick / 60000.0);
    //stateMachineValues.Fr = qPow(10, -0.1 * stateMachineValues.dTemp) * (Globals::stateMachineTick / 60000.0);
    //stateMachineValues.r = 5 * stateMachineValues.Fr;

    updateStateMachineValues.Dr = k * d0 * qPow(10, -1*exp) * (Globals::stateMachineTick / 60000.0);
    updateStateMachineValues.Fr = (qPow(10, exp) * (Globals::stateMachineTick / 60000.0)) / (k * d0);
    updateStateMachineValues.r = (qPow(10, exp) * (Globals::stateMachineTick / 60000.0)) / d0 ;

    updateStateMachineValues.state = state;

    if (isRunning()) {
        updateStateMachineValues.time = processStart.msecsTo(QDateTime::currentDateTime());
        updateStateMachineValues.sumFr = stateMachineValues.sumFr + updateStateMachineValues.Fr;
        updateStateMachineValues.sumr = stateMachineValues.sumr + updateStateMachineValues.r;
    } else {
        updateStateMachineValues.time = 0;
        updateStateMachineValues.sumFr = 0;
        updateStateMachineValues.sumr = 0;
    }

    return updateStateMachineValues;
}


StateMachineValues StateMachine::calculateDrFrRValuesAndUpdateDbFromSensors(int processId)
{
    auto stateMachineValues = calculateStateMachineValues();


    return stateMachineValues;
}

void StateMachine::autoklavControl()
{
    //tankControl();
    //verificationControl();
    //pipeControl();

    stateMachineValues = calculateStateMachineValues();
    DbManager::instance().createProcessLog(process->getId());

    switch (state) {
    case State::READY:
        break;

    case State::STARTING:
        Logger::info("StateMachine: Starting");

        Sensor::mapName[CONSTANTS::AUTOKLAV_FILL]->send(1);
        stopwatch1 = QDateTime::currentDateTime().addSecs(32); // 3 minutes

        state = State::FILLING;
        Logger::info("StateMachine: Filling");
        break;

    case State::FILLING:

        if(QDateTime::currentDateTime() < stopwatch1){
            Logger::info("Wait 3min");
            break;
        }

        Sensor::mapName[CONSTANTS::PUMP]->send(1);
        Sensor::mapName[CONSTANTS::HEATING]->send(1);

        if(stateMachineValues.pressure < 0.1)
            break;

        Sensor::mapName[CONSTANTS::AUTOKLAV_FILL]->send(0);
        Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(1);

        if(stateMachineValues.pressure < 1)
            break;


        Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(0);

        state = State::HEATING;
        Logger::info("StateMachine: Heating");
        break;

    case State::HEATING:

        if(stateMachineValues.temp < processConfig.maintainTemp)
            break;

        state = State::STERILIZING;
        heatingStart =  QDateTime::currentDateTime();
        Logger::info("StateMachine: Sterilizing");
        break;

    case State::STERILIZING:

        if (stateMachineValues.temp > processConfig.maintainTemp + 1) {
            Sensor::mapName[CONSTANTS::HEATING]->send(0);
        } else if (stateMachineValues.temp < processConfig.maintainTemp - 1) {
            Sensor::mapName[CONSTANTS::HEATING]->send(1);
        }

        if (processConfig.mode == Mode::TARGETF) {            
            if (stateMachineValues.sumFr < processInfo.targetF.toDouble()){
                Logger::info("Wait until sumfr is reached");
                break;
            }
        } else if (processConfig.mode == Mode::TIME) {
            if (heatingStart.msecsTo(QDateTime::currentDateTime()) < processInfo.targetHeatingTime.toDouble()){
                Logger::info("Wait until time is reached");
                break;
            }
        }

        heatingTime = heatingStart.msecsTo(QDateTime::currentDateTime());

        Sensor::mapName[CONSTANTS::HEATING]->send(0);
        Sensor::mapName[CONSTANTS::COOLING]->send(1);
        Sensor::mapName[CONSTANTS::COOLING_HELPER]->send(1);
        Sensor::mapName[CONSTANTS::FILL_TANK_WITH_WATER]->send(1);

        state = State::COOLING;
        Logger::info("StateMachine: Cooling");
        coolingStart = QDateTime::currentDateTime();
        break;

    case State::COOLING:

        if(stateMachineValues.tankWaterLevel < 80) {
            Logger::info("Wait until tank water level is reached");
            break;
        }

        Sensor::mapName[CONSTANTS::FILL_TANK_WITH_WATER]->send(0);

        if (processConfig.mode == Mode::TARGETF) {

            if (stateMachineValues.tempK > Globals::coolingThreshold)
                break;

            Sensor::mapName[CONSTANTS::COOLING]->send(0);
            Sensor::mapName[CONSTANTS::PUMP]->send(0);

            if(stateMachineValues.tempK > processConfig.finishTemp)
                break;

            Sensor::mapName[CONSTANTS::COOLING_HELPER]->send(0);

        } else if (processConfig.mode == Mode::TIME) {
            if (coolingStart.msecsTo(QDateTime::currentDateTime()) < processInfo.targetCoolingTime.toDouble()) {
                Logger::info("Wait until target cooling is reached");
                break;
            }

            Sensor::mapName[CONSTANTS::COOLING]->send(0);
            Sensor::mapName[CONSTANTS::PUMP]->send(0);

            //if(coolingStart.msecsTo(QDateTime::currentDateTime()) < processInfo.targetCoolingTime.toDouble())
                //break;

            Sensor::mapName[CONSTANTS::COOLING_HELPER]->send(0);
        }

        Sensor::mapName[CONSTANTS::WATER_DRAIN]->send(1); // ignoriramo alarm otvorena vrata

        state = State::FINISHING;
        coolingTime = coolingStart.msecsTo(QDateTime::currentDateTime());
        stopwatch1 = QDateTime::currentDateTime().addSecs(32); // 10 minutes
        Logger::info("StateMachine: Cooling helper");        
        break;

    case State::FINISHING:

        if(QDateTime::currentDateTime() < stopwatch1){
            Logger::info("Wait 10min");
            break;
        }

        Sensor::mapName[CONSTANTS::WATER_DRAIN]->send(0);

        state = State::FINISHED;
        Logger::info("StateMachine: Finished");
        break;

    case State::FINISHED:
        timer->stop();

        Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(1);
        Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(0);
        Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(1);
        Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(0);

        Logger::info("StateMachine: Ready");

        if (process) {
            auto processInfo = process->getInfo();
            processInfo.processLength = QString::number(stateMachineValues.time);
            processInfo.targetCoolingTime = QString::number(coolingTime);
            processInfo.targetHeatingTime = QString::number(heatingTime);
            process->setInfo(processInfo);
        }

        state = State::READY;
        stateMachineValues = StateMachineValues();
        break;
    }
}

bool StateMachine::stopManualMeasuring(){

    if (!isRunning())
        return false;

    if (process) {
        auto processInfo = process->getInfo();
        processInfo.processLength = QString::number(stateMachineValues.time);
        processInfo.targetCoolingTime = QString::number(0);
        processInfo.targetHeatingTime = QString::number(0);
        process->setInfo(processInfo);
    }

    state = State::READY;

    return true;
}

StateMachine &StateMachine::instance()
{
    static StateMachine _instance;
    return _instance;
}
