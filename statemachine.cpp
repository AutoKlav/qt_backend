#include "statemachine.h"

#include "logger.h"
#include "sensor.h"
#include "globals.h"
#include "dbmanager.h"
#include "constants.h"
#include "globalerrors.h"

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
    // TODO uncomment this on new version
    //tankControl();
    autoklavControl();
}

void StateMachine::triggerAlarm()
{
    Logger::warn("Alarm triggered!");
    Sensor::mapOutputPin[CONSTANTS::ALARM_SIGNAL]->send(1);
    Sensor::mapOutputPin[CONSTANTS::ALARM_SIGNAL]->send(0);
}

void StateMachine::pipeControl()
{
    if (stateMachineValues.expansionTemp > Globals::expansionUpperTemp) {
        Sensor::mapOutputPin[CONSTANTS::EXTENSION_COOLING]->sendIfNew(1);
        Logger::info(QString("Pipe cooling on, expansionTemp = %1 > %2").arg(stateMachineValues.expansionTemp).arg(Globals::expansionUpperTemp));
    }
    else if (stateMachineValues.expansionTemp < Globals::expansionLowerTemp) {
        Sensor::mapOutputPin[CONSTANTS::EXTENSION_COOLING]->sendIfNew(0);
        Logger::info(QString("Pipe cooling off, expansionTemp = %1 < %2").arg(stateMachineValues.expansionTemp).arg(Globals::expansionLowerTemp));
    }
}

void StateMachine::tankControl()
{   
    // When acceptable water level inside tank to turn on heaters
    if (stateMachineValues.tankWaterLevel > Globals::heaterWaterLevel) {

        // Maintain temp of water inside tank ±1
        if (stateMachineValues.tankTemp > Globals::maintainWaterTankTemp + 1) {
            Sensor::mapOutputPin[CONSTANTS::TANK_HEATING]->sendIfNew(0);
            Logger::info(QString("Tank heating off, tankTemp = %1 > %2").arg(stateMachineValues.tankTemp).arg(Globals::maintainWaterTankTemp));
            
        } else if (stateMachineValues.tankTemp < Globals::maintainWaterTankTemp - 1) {
            if (isRunning()) { // Turn on heaters only while process is running            
                Sensor::mapOutputPin[CONSTANTS::TANK_HEATING]->sendIfNew(1);
                Logger::info(QString("Tank heating on, tankTemp = %1 < %2").arg(stateMachineValues.tankTemp).arg(Globals::maintainWaterTankTemp));
            }
        }
    } else {
        Sensor::mapOutputPin[CONSTANTS::TANK_HEATING]->sendIfNew(0);
        Logger::info(QString("Tank heating off, tankWaterLevel = %1 <= %2").arg(stateMachineValues.tankWaterLevel).arg(Globals::heaterWaterLevel));
    }

    if (stateMachineValues.tankWaterLevel > 100 && state != State::COOLING) {
        Sensor::mapOutputPin[CONSTANTS::FILL_TANK_WITH_WATER]->sendIfNew(0);
        Logger::info(QString("Tank filling off, tankWaterLevel >= 100 (%1) and not cooling").arg(stateMachineValues.tankWaterLevel));
    }
}

bool StateMachine::start(ProcessConfig processConfig, ProcessInfo processInfo)
{
    Logger::info("Process starting");

    if (isRunning()){
        Logger::warn("Start failed: Autoklav is already running");
        GlobalErrors::setError(GlobalErrors::AlreadyStarted);
        return false;
    }
    else {
        GlobalErrors::removeError(GlobalErrors::AlreadyStarted);
    }

    if (!verificationControl()) {
        Logger::warn("Start failed");
        return false;
    }

    this->processConfig = processConfig;
    this->processInfo = processInfo;
    state = State::STARTING;

    processStart = QDateTime::currentDateTime();    

    // Fetch first time values and abort start if door is not closed
    stateMachineValues = calculateStateMachineValues();    

    process = new Process(processStart.toString(Qt::ISODate), processInfo, this);

    Logger::info("Process started");

    tick();
    timer.start(Globals::stateMachineTick);
    writeInDBstopwatch = QDateTime::currentDateTime().addMSecs(Globals::dbTick);


    // clear hardcoded ending values, empty string ("") can be used as well
    heatingEnd.clear();
    coolingEnd.clear();

    return true;
}

bool StateMachine::stop()
{
    Logger::info("End process");
    timer.stop();

    if (process) {
        auto processInfo = process->getInfo();
        processInfo.processLength = QString::number(stateMachineValues.time);        
        process->setInfo(processInfo);
    }

    Sensor::mapOutputPin[CONSTANTS::FILL_TANK_WITH_WATER]->send(0);
    Sensor::mapOutputPin[CONSTANTS::COOLING]->send(0);
    Sensor::mapOutputPin[CONSTANTS::TANK_HEATING]->send(0);
    Sensor::mapOutputPin[CONSTANTS::COOLING_HELPER]->send(0);
    Sensor::mapOutputPin[CONSTANTS::AUTOKLAV_FILL]->send(0);
    Sensor::mapOutputPin[CONSTANTS::WATER_DRAIN]->send(0);
    Sensor::mapOutputPin[CONSTANTS::STEAM_HEATING]->send(0);
    Sensor::mapOutputPin[CONSTANTS::PUMP]->send(0);
    Sensor::mapOutputPin[CONSTANTS::ELECTRIC_HEATING]->send(0);
    Sensor::mapOutputPin[CONSTANTS::INCREASE_PRESSURE]->send(0);
    Sensor::mapOutputPin[CONSTANTS::EXTENSION_COOLING]->send(0);
    Sensor::mapOutputPin[CONSTANTS::ALARM_SIGNAL]->send(0);

    state = State::READY;

    // clear hardcoded ending values, empty string ("") can be used as well
    heatingEnd.clear();
    coolingEnd.clear();

    stateMachineValues = StateMachineValues();

    return true;
}

inline bool StateMachine::isRunning()
{
    return state != State::READY;
}

StateMachineValues StateMachine::getValues()
{
    // If Process is not started, display only input pin values
    if(!isRunning()){
        return readInputPinValues();
    }

    return stateMachineValues;
}

// This method is reused for displaying input values before and after process starts but only common data
StateMachineValues StateMachine::readInputPinValues()
{
    StateMachineValues updateStateMachineValues = {};

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

    return updateStateMachineValues;    
}

StateMachineValues StateMachine::calculateStateMachineValues()
{
    StateMachineValues updateStateMachineValues = readInputPinValues();    

    updateStateMachineValues.state = state;

    const auto k = Globals::k;
    const auto z = processInfo.bacteria.z;
    const auto d0 = processInfo.bacteria.d0;

    if (processConfig.mode == Mode::TARGETF) {
        updateStateMachineValues.dTemp = updateStateMachineValues.tempK - processInfo.processType.customTemp;

    } else if (processConfig.mode == Mode::TIME) {
        // avoid calculations it time is selected
        updateStateMachineValues.dTemp = NAN;
    }

    updateStateMachineValues.Dr = k * d0 * qPow(10, -1.0 / z * updateStateMachineValues.dTemp);
    updateStateMachineValues.Fr = 1.0 / (k * d0) * qPow(10, 1.0 / z * updateStateMachineValues.dTemp);
    updateStateMachineValues.r = 1.0 / d0 * qPow(10, 1.0 / z * updateStateMachineValues.dTemp);


    updateStateMachineValues.time = processStart.msecsTo(QDateTime::currentDateTime()) / 1000.0; // in seconds
    updateStateMachineValues.sumFr = stateMachineValues.sumFr + updateStateMachineValues.Fr * (Globals::stateMachineTick / 60000.0);
    updateStateMachineValues.sumr = stateMachineValues.sumr + updateStateMachineValues.r * (Globals::stateMachineTick / 60000.0);

    //const double tickFactor = Globals::stateMachineTick / 60000.0;
    //const double tempFactor = qPow(10, 1.0 / z * updateStateMachineValues.dTemp);
    
    // qDebug() << "###";
    // qDebug() << "tempK: " << updateStateMachineValues.tempK;
    // qDebug() << "customTemp: " << processConfig.customTemp;
    // qDebug() << "dtemp: tempK - customTemp:" << updateStateMachineValues.dTemp;
    // qDebug() << "k:" << k;
    // qDebug() << "z:" << z;
    // qDebug() << "d0:" << d0;
    // qDebug() << "stateMachineTick:" << Globals::stateMachineTick;
    // qDebug() << "tickFactor: (stateMachineTick/60000):" << tickFactor;
    // qDebug() << "tempFactor: (10^(1/z * dTemp)):" << tempFactor;
    // qDebug() << "Formula for Dr: k * d0 * 10^(-1.0/z * dTemp) * (stateMachineTick / 60000)";
    // qDebug() << "Dr:" << updateStateMachineValues.Dr;
    // qDebug() << "Formula for Fr: (1.0 / (k * d0)) * 10^(1.0/z * dTemp) * (stateMachineTick / 60000)";
    // qDebug() << "Fr:" << updateStateMachineValues.Fr;
    // qDebug() << "Formula for r: (1.0 / d0) * 10^(1.0/z * dTemp) * (stateMachineTick / 60000)";
    // qDebug() << "r:" << updateStateMachineValues.r;
    // qDebug() << "time:" << QDateTime::currentDateTime();
    // qDebug() << "sumFr:" << updateStateMachineValues.sumFr;
    // qDebug() << "sumr:" << updateStateMachineValues.r;
    // qDebug() << "###";

    return updateStateMachineValues;
}


bool StateMachine::verificationControl()
{
    // TODO Remove this
    return true;
    auto sensorValues = Sensor::getValues();

    auto turnOnAlarm = false;
    if (!sensorValues.doorClosed) {
        GlobalErrors::setError(GlobalErrors::DoorClosedError);
        Logger::warn("Door not closed!");
        turnOnAlarm = true;
    }
    else {
        GlobalErrors::removeError(GlobalErrors::DoorClosedError);
    }

    if (sensorValues.burnerFault) {
        GlobalErrors::setError(GlobalErrors::BurnerError);
        Logger::warn("Burner fault");
        turnOnAlarm = true;
    }
    else {
        GlobalErrors::removeError(GlobalErrors::BurnerError);
    }

    if (sensorValues.waterShortage) {
        GlobalErrors::setError(GlobalErrors::WaterShortageError);
        Logger::warn("Water shortage in burner fault");
        turnOnAlarm = true;
    }
    else {
        GlobalErrors::removeError(GlobalErrors::WaterShortageError);
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

        Sensor::mapOutputPin[CONSTANTS::AUTOKLAV_FILL]->send(1);
        stopwatch1 = QDateTime::currentDateTime().addMSecs(3*60*1000); // 10 minutes TODO revert for testing to 1000

        state = State::FILLING;
        Logger::info("StateMachine: Filling");
        break;

    case State::FILLING:

        if(QDateTime::currentDateTime() < stopwatch1){
            Logger::info("Wait 3min");
            break;
        }

        Sensor::mapOutputPin[CONSTANTS::PUMP]->send(1);
        Sensor::mapOutputPin[CONSTANTS::STEAM_HEATING]->send(1);


        if(stateMachineValues.pressure < 0.16){
            Logger::info(QString("Wait until pressure %1 reaches 0.16").arg(QString::number(stateMachineValues.pressure)));
            break;
        }

        Sensor::mapOutputPin[CONSTANTS::AUTOKLAV_FILL]->send(0);
        Sensor::mapOutputPin[CONSTANTS::INCREASE_PRESSURE]->send(1);

        if(stateMachineValues.pressure < 1.5){
            Logger::info(QString("Wait until pressure %1 reaches 1.5").arg(QString::number(stateMachineValues.pressure)));
            break;
        }
           
        Sensor::mapOutputPin[CONSTANTS::INCREASE_PRESSURE]->send(0);

        state = State::HEATING;
                
        Logger::info("StateMachine: Heating");
        break;

    case State::HEATING:

        if(stateMachineValues.temp < processInfo.processType.maintainTemp) {
            Logger::info(QString("Wait until %1 reaches %2").arg(QString::number(stateMachineValues.temp)).arg(QString::number(processInfo.processType.maintainTemp)));
            break; // TODO revert for testing
        }

        state = State::STERILIZING;
        heatingStart = QDateTime::currentDateTime();
        heatingEnd = (heatingStart.addMSecs(static_cast<qint64>(processInfo.targetHeatingTime.toDouble()))).toString(Qt::ISODate);

        Logger::info("StateMachine: Sterilizing");
        break;

    case State::STERILIZING:

        if (stateMachineValues.temp > processInfo.processType.maintainTemp + 0.5) {
            Sensor::mapOutputPin[CONSTANTS::STEAM_HEATING]->send(0);
        } else if (stateMachineValues.temp < processInfo.processType.maintainTemp - 0.5) {
            Sensor::mapOutputPin[CONSTANTS::STEAM_HEATING]->send(1);
        }        

        if (processConfig.mode == Mode::TARGETF) {            
            if (stateMachineValues.sumFr < processInfo.targetF.toDouble()) {
                Logger::info(QString("Wait until sumfr %1 is reached").arg(QString::number(processInfo.targetF.toDouble())));
                break;
            }
        } else if (processConfig.mode == Mode::TIME) {
            if (heatingStart.msecsTo(QDateTime::currentDateTime()) < processInfo.targetHeatingTime.toDouble()) {
                Logger::info(QString("Wait until target time %1 is reached").arg(QString::number(processInfo.targetHeatingTime.toDouble())));
                                
                coolingStart = QDateTime::currentDateTime();

                break;
            }
        
        }

        state = State::PRECOOLING;
        Logger::info("StateMachine: Pre cooling");
        break;

    case State::PRECOOLING:

        Sensor::mapOutputPin[CONSTANTS::STEAM_HEATING]->send(0);
        Sensor::mapOutputPin[CONSTANTS::COOLING]->send(1);
        Sensor::mapOutputPin[CONSTANTS::COOLING_HELPER]->send(1);
        Sensor::mapOutputPin[CONSTANTS::FILL_TANK_WITH_WATER]->send(1);

        if(stateMachineValues.tankWaterLevel < Globals::tankWaterLevelThreshold) {
            Logger::info(QString("Wait until tank water level %1 is reached").arg(Globals::tankWaterLevelThreshold));
            break;
        }

        state = State::COOLING;
        coolingEnd = (coolingStart.addMSecs(static_cast<qint64>(processInfo.targetCoolingTime.toDouble()))).toString(Qt::ISODate);

        Logger::info("StateMachine: Cooling");
        break;

    case State::COOLING:        

        Sensor::mapOutputPin[CONSTANTS::COOLING]->send(0);
        Sensor::mapOutputPin[CONSTANTS::FILL_TANK_WITH_WATER]->send(0);

        if (processConfig.mode == Mode::TARGETF) {
            if(stateMachineValues.tempK > processInfo.finishTemp.toDouble()) {
                Logger::info(QString("Wait until tempK %1 is reached").arg(QString::number(processInfo.finishTemp.toDouble())));
                break;
            }

        } else if (processConfig.mode == Mode::TIME) {
            if (coolingStart.msecsTo(QDateTime::currentDateTime()) < processInfo.targetCoolingTime.toDouble()) {
                Logger::info("Wait until target cooling is reached");
                break;
            }
        }

        Sensor::mapOutputPin[CONSTANTS::COOLING_HELPER]->send(0);
        Sensor::mapOutputPin[CONSTANTS::PUMP]->send(0);
        Sensor::mapOutputPin[CONSTANTS::WATER_DRAIN]->send(1);

        state = State::FINISHING;
        stopwatch1 = QDateTime::currentDateTime().addMSecs(10*60*1000); // 10 minutes
        Logger::info("StateMachine: Finishing");
        break;

    case State::FINISHING:

        if(QDateTime::currentDateTime() < stopwatch1){
            Logger::info("Wait 10min");
            break;
        }

        Sensor::mapOutputPin[CONSTANTS::WATER_DRAIN]->send(0);
        Sensor::mapOutputPin[CONSTANTS::EXTENSION_COOLING]->sendIfNew(0);
        Sensor::mapOutputPin[CONSTANTS::TANK_HEATING]->sendIfNew(0);
        Sensor::mapOutputPin[CONSTANTS::FILL_TANK_WITH_WATER]->sendIfNew(0);

        state = State::FINISHED;
        Logger::info("StateMachine: Finished");
        break;

    case State::FINISHED:
        timer.stop();

        Sensor::mapOutputPin[CONSTANTS::ALARM_SIGNAL]->send(1);
        Sensor::mapOutputPin[CONSTANTS::ALARM_SIGNAL]->send(0);
        Sensor::mapOutputPin[CONSTANTS::ALARM_SIGNAL]->send(1);
        Sensor::mapOutputPin[CONSTANTS::ALARM_SIGNAL]->send(0);

        Logger::info("StateMachine: Ready");

        if (process) {
            auto processInfo = process->getInfo();
            processInfo.processLength = QString::number(stateMachineValues.time);            
            process->setInfo(processInfo);
        }

        state = State::READY;

        // clear hardcoded ending values, empty string ("") can be used as well
        heatingEnd.clear();
        coolingEnd.clear();

        stateMachineValues = StateMachineValues();
        break;
    }
}

StateMachine &StateMachine::instance()
{
    static StateMachine _instance;
    return _instance;
}
