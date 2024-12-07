#include "globals.h"

#include "dbmanager.h"

bool Globals::setSerialDataTime(int value)
{
    if (serialDataTime == value)
        return true;

    auto& db = DbManager::instance();
    if (db.updateGlobal("serialDataTime", QString::number(value))) {
        serialDataTime = value;
        return true;
    }

    return false;
}

bool Globals::setStateMachineTick(int value)
{
    if (stateMachineTick == value)
        return true;

    auto& db = DbManager::instance();
    if (db.updateGlobal("stateMachineTick", QString::number(value))) {
        stateMachineTick = value;
        return true;
    }

    return false;
}

bool Globals::setSterilizationTemp(double value)
{
    if (sterilizationTemp == value)
        return true;

    auto& db = DbManager::instance();
    if (db.updateGlobal("sterilizationTemp", QString::number(value))) {
        sterilizationTemp = value;
        return true;
    }

    return false;
}

bool Globals::setPasterizationTemp(double value)
{
    if (pasterizationTemp == value)
        return true;

    auto& db = DbManager::instance();
    if (db.updateGlobal("pasterizationTemp", QString::number(value))) {
        pasterizationTemp = value;
        return true;
    }

    return false;
}

Globals::Variables Globals::getVariables()
{
    return {
        serialDataTime, stateMachineTick, sterilizationTemp, pasterizationTemp
    };
}
