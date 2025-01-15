#include "master.h"

#include "logger.h"
#include "serial.h"
#include "dbmanager.h"
//#include "mockserial.h"
#include "statemachine.h"

Master::Master(QObject *parent)
    : QObject{parent}
{
    auto &db = DbManager::instance();
    db.loadGlobals();
    db.loadSensors();

    Serial::instance().open();

    Sensor::requestRelayUpdate();

    StateMachine::instance();

    Logger::info("Program started");
}

Master &Master::instance()
{
    static Master _instance{};
    return _instance;
}
