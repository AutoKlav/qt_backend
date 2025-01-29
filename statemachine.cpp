#include "statemachine.h"

#include "logger.h"
#include "sensor.h"
#include "globals.h"
#include "dbmanager.h"
#include "constants.h"

// Constructor
StateMachine::StateMachine(QObject *parent)
    : QObject(parent), state(READY), process(nullptr)
{
    connect(&timer, &QTimer::timeout, this, &StateMachine::tick);
}

int StateMachine::getState()
{
    return static_cast<int>(state);
}

void StateMachine::tick()
{
    pipeControl();
    tankControl();
    autoklavControl();
}

void StateMachine::triggerAlarm()
{
    Logger::warn("Alarm triggered!");
    Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(1);
    Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(0);    
}

void StateMachine::selectAndActivateHeatingType(uint sendState)
{
    if(processConfig.heatingType == StateMachine::ELECTRIC)
        Sensor::mapName[CONSTANTS::ELECTRIC_HEATING]->send(sendState);

    else if(processConfig.heatingType == StateMachine::STEAM)
        Sensor::mapName[CONSTANTS::STEAM_HEATING]->send(sendState);

    else if(processConfig.heatingType == StateMachine::STEAM_ELECTRIC)
    {
        Sensor::mapName[CONSTANTS::ELECTRIC_HEATING]->send(sendState);
        Sensor::mapName[CONSTANTS::STEAM_HEATING]->send(sendState);
    }
}

void StateMachine::pipeControl()
{
    if (stateMachineValues.expansionTemp > Globals::expansionUpperTemp) {
        Sensor::mapName[CONSTANTS::EXTENSION_COOLING]->sendIfNew(1);
    }
    else if (stateMachineValues.expansionTemp < Globals::expansionLowerTemp) {
        Sensor::mapName[CONSTANTS::EXTENSION_COOLING]->sendIfNew(0);
    }
}

void StateMachine::tankControl()
{   
    // When acceptable water level inside tank to turn on heaters
    if (stateMachineValues.tankWaterLevel > Globals::heaterWaterLevel) {

        // Maintain temp of water inside tank ±1
        if (stateMachineValues.tankTemp > Globals::maintainWaterTankTemp + 1) {
            Sensor::mapName[CONSTANTS::TANK_HEATING]->sendIfNew(0);
        } else if (stateMachineValues.tankTemp < Globals::maintainWaterTankTemp - 1) {
            if (isRunning()) // Turn on heaters only while process is running
                Sensor::mapName[CONSTANTS::TANK_HEATING]->sendIfNew(1);
        }
    } else {
        Sensor::mapName[CONSTANTS::TANK_HEATING]->sendIfNew(0);
    }

    if (stateMachineValues.tankWaterLevel > 100 && state != State::COOLING) {
        Sensor::mapName[CONSTANTS::FILL_TANK_WITH_WATER]->sendIfNew(0);
    }
}

bool StateMachine::start(ProcessConfig processConfig, ProcessInfo processInfo)
{
    Logger::info("Process starting");

    if (isRunning()){
        Logger::warn("Start failed: Autoklav is already running");
        return false;
    }

    // Fetch first time values and abort start if door is not closed
    stateMachineValues = calculateStateMachineValues();

    if (!verificationControl()) {
        Logger::warn("Start failed");
        return false;
    }

    this->processConfig = processConfig;
    this->processInfo = processInfo;
    state = State::STARTING;

    processStart = QDateTime::currentDateTime();
    process = new Process(processStart.toString(Qt::ISODate), processInfo, this);

    Logger::info("Process started");

    tick();
    timer.start(Globals::stateMachineTick);
    writeInDBstopwatch = QDateTime::currentDateTime().addMSecs(Globals::dbTick);

    return true;
}

bool StateMachine::stop()
{
    if (!isRunning())
        return false;

    Logger::info("End process");
    timer.stop();

    Sensor::mapName[CONSTANTS::FILL_TANK_WITH_WATER]->send(0);
    Sensor::mapName[CONSTANTS::COOLING]->send(0);
    Sensor::mapName[CONSTANTS::TANK_HEATING]->send(0);
    Sensor::mapName[CONSTANTS::COOLING_HELPER]->send(0);
    Sensor::mapName[CONSTANTS::AUTOKLAV_FILL]->send(0);
    Sensor::mapName[CONSTANTS::WATER_DRAIN]->send(0);    
    selectAndActivateHeatingType(0);
    Sensor::mapName[CONSTANTS::PUMP]->send(0);
    Sensor::mapName[CONSTANTS::ELECTRIC_HEATING]->send(0);
    Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(0);
    Sensor::mapName[CONSTANTS::EXTENSION_COOLING]->send(0);
    Sensor::mapName[CONSTANTS::ALARM_SIGNAL]->send(0);

    state = State::READY;    
    stateMachineValues = StateMachineValues();

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

    updateStateMachineValues.dTemp = updateStateMachineValues.tempK - processConfig.customTemp;

    const auto k = Globals::k;
    const auto z = processInfo.bacteria.z;
    const auto d0 = processInfo.bacteria.d0;

    updateStateMachineValues.Dr = k * d0 * qPow(10, -1.0/z*updateStateMachineValues.dTemp) * (Globals::stateMachineTick / 60000.0);
    updateStateMachineValues.Fr = 1.0 / (k * d0) * qPow(10, 1.0/z*updateStateMachineValues.dTemp) * (Globals::stateMachineTick / 60000.0);
    updateStateMachineValues.r = 1.0 / d0 * qPow(10, 1.0/z*updateStateMachineValues.dTemp) * (Globals::stateMachineTick / 60000.0);

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


bool StateMachine::verificationControl()
{
    auto turnOnAlarm = false;
    if (!stateMachineValues.doorClosed) {
        Logger::warn("Door not closed!");
        turnOnAlarm = true;
    }

    if (stateMachineValues.burnerFault) {
        Logger::warn("Burner fault");
        turnOnAlarm = true;
    }

    if (stateMachineValues.waterShortage) {
        Logger::warn("Water shortage in burner fault");
        turnOnAlarm = true;
    }

    if (turnOnAlarm)
        triggerAlarm();

    return !turnOnAlarm;
}


void StateMachine::autoklavControl()
{    
    stateMachineValues = calculateStateMachineValues();

    if(QDateTime::currentDateTime() > writeInDBstopwatch) {
        DbManager::instance().createProcessLog(process->getId());
        writeInDBstopwatch = QDateTime::currentDateTime().addMSecs(Globals::dbTick);
    }


    if(!verificationControl()) {
        Logger::warn("Verification failed, not doing anything!");
    }

    switch (state) {
    case State::READY:
        break;

    case State::STARTING:
        Logger::info("StateMachine: Starting");

        Sensor::mapName[CONSTANTS::AUTOKLAV_FILL]->send(1);
        stopwatch1 = QDateTime::currentDateTime().addMSecs(3*60*1000); // 3 minutes

        state = State::FILLING;
        Logger::info("StateMachine: Filling");
        break;

    case State::FILLING:

        if(QDateTime::currentDateTime() < stopwatch1){
            Logger::info("Wait 3min");
            break;
        }

        Sensor::mapName[CONSTANTS::PUMP]->send(1);
        selectAndActivateHeatingType(1);


        if(stateMachineValues.pressure < 0.16)
            break;

        Sensor::mapName[CONSTANTS::AUTOKLAV_FILL]->send(0);
        Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(1);

        if(stateMachineValues.pressure < 1.5)
            break;

        Sensor::mapName[CONSTANTS::INCREASE_PRESSURE]->send(0);

        state = State::HEATING;
        Logger::info("StateMachine: Heating");
        break;

    case State::HEATING:

        if(stateMachineValues.temp < processConfig.maintainTemp) {
            Logger::info(QString("Wait until %1 reaches %2").arg(QString::number(stateMachineValues.temp)).arg(QString::number(processConfig.maintainTemp)));
            break;
        }

        state = State::STERILIZING;
        heatingStart =  QDateTime::currentDateTime();
        Logger::info("StateMachine: Sterilizing");
        break;

    case State::STERILIZING:

        if (stateMachineValues.temp > processConfig.maintainTemp + 0.5) {            
            selectAndActivateHeatingType(0);
        } else if (stateMachineValues.temp < processConfig.maintainTemp - 0.5) {
            selectAndActivateHeatingType(1);
        }        

        if (processConfig.mode == Mode::TARGETF) {            
            if (stateMachineValues.sumFr < processInfo.targetF.toDouble()){
                Logger::info("Wait until sumfr is reached");
                break;
            }
        } else if (processConfig.mode == Mode::TIME) {
            if (heatingStart.msecsTo(QDateTime::currentDateTime()) < processInfo.targetHeatingTime.toDouble()){
                Logger::info("Wait until target time is reached");
                break;
            }
        }

        heatingTime = heatingStart.msecsTo(QDateTime::currentDateTime());

        selectAndActivateHeatingType(0);
        Sensor::mapName[CONSTANTS::COOLING]->send(1);
        Sensor::mapName[CONSTANTS::COOLING_HELPER]->send(1);
        Sensor::mapName[CONSTANTS::FILL_TANK_WITH_WATER]->send(1);

        state = State::COOLING;
        Logger::info("StateMachine: Cooling");
        coolingStart = QDateTime::currentDateTime();
        break;

    case State::COOLING:
        if(stateMachineValues.tankWaterLevel < 95) {
            Logger::info("Wait until tank water level is reached");
            break;
        }

        Sensor::mapName[CONSTANTS::COOLING]->send(0);
        Sensor::mapName[CONSTANTS::FILL_TANK_WITH_WATER]->send(0);

        if (processConfig.mode == Mode::TARGETF) {
            if(stateMachineValues.tempK > processConfig.finishTemp) {
                Logger::info("Wait until tempK reaches finish temp");
                break;
            }

        } else if (processConfig.mode == Mode::TIME) {
            if (coolingStart.msecsTo(QDateTime::currentDateTime()) < processInfo.targetCoolingTime.toDouble()) {
                Logger::info("Wait until target cooling is reached");
                break;
            }
        }

        Sensor::mapName[CONSTANTS::COOLING_HELPER]->send(0);
        Sensor::mapName[CONSTANTS::PUMP]->send(0);
        Sensor::mapName[CONSTANTS::WATER_DRAIN]->send(1);

        state = State::FINISHING;
        coolingTime = coolingStart.msecsTo(QDateTime::currentDateTime());
        stopwatch1 = QDateTime::currentDateTime().addMSecs(10*60*1000); // 10 minutes
        Logger::info("StateMachine: Finishing");
        break;

    case State::FINISHING:

        if(QDateTime::currentDateTime() < stopwatch1){
            Logger::info("Wait 10min");
            break;
        }

        Sensor::mapName[CONSTANTS::WATER_DRAIN]->send(0);
        Sensor::mapName[CONSTANTS::EXTENSION_COOLING]->sendIfNew(0);
        Sensor::mapName[CONSTANTS::TANK_HEATING]->sendIfNew(0);
        Sensor::mapName[CONSTANTS::FILL_TANK_WITH_WATER]->sendIfNew(0);

        state = State::FINISHED;
        Logger::info("StateMachine: Finished");
        break;

    case State::FINISHED:
        timer.stop();

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

bool StateMachine::setState(uint newState)
{
    // Check if newState is within the valid range of the enum
    if (newState >= READY && newState <= FINISHED) {
        this->state = static_cast<State>(newState);
        return true;  // State change successful
    }

    return false;  // Invalid state, return false
}

StateMachine &StateMachine::instance()
{
    static StateMachine _instance;
    return _instance;
}
