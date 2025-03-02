#include "master.h"
#include "modbus.h"
#include "logger.h"
#include "dbmanager.h"
#include "statemachine.h"

Master::Master(QObject *parent)
    : QObject{parent}
{
    auto &db = DbManager::instance();
    db.loadGlobals();
    db.loadAnalogSensors();
    db.loadDigitalSensors();

    //Serial::instance().open();
    Modbus &modbusApp = Modbus::instance();

    modbusApp.connectToServer("172.16.0.2", 502);


    StateMachine::instance();

    Logger::info("Program started");
}

Master &Master::instance()
{
    static Master _instance{};
    return _instance;
}
