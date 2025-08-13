#include "master.h"
#include "logger.h"
#include "dbmanager.h"
#include "statemachine.h"
#include "modbusrtu.h"

Master::Master(QObject *parent)
    : QObject{parent}
{
    auto &db = DbManager::instance();
    db.loadGlobals();
    db.loadInputPins();
    db.loadOutputPins();

    //Modbus &modbusApp = Modbus::instance();

    //modbusApp.connectToServer("172.16.0.2", 502);

    // Initialize Modbus RTU
    ModbusRTU &rtu = ModbusRTU::instance();
    rtu.connectToDevice();

    StateMachine::instance();

    Logger::info("Program started");
}

Master &Master::instance()
{
    static Master _instance{};
    return _instance;
}
