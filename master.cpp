#include "master.h"

#include "logger.h"
#include "dbmanager.h"
#include "mockserial.h"
#include "statemachine.h"
#include "serial.h"

Master::Master(QObject *parent)
    : QObject{parent}
{
    auto &db = DbManager::instance();

    db.loadGlobals();
    db.loadSensors();

    // Switch to MockSerial to test without Arduino
    Serial::instance();
//    MockSerial::instance();

    StateMachine::instance();

    Logger::info("Program started");
}

Master &Master::instance()
{
    static Master _instance{};
    return _instance;
}
