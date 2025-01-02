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

bool Globals::setK(double value)
{
    if (k == value)
        return true;

    auto& db = DbManager::instance();
    if (db.updateGlobal("k", QString::number(value))) {
        k = value;
        return true;
    }

    return false;
}

bool Globals::setCoolingThreshold(double value)
{
    if (coolingThreshold == value)
        return true;

    auto& db = DbManager::instance();
    if (db.updateGlobal("coolingThreshold", QString::number(value))) {
        coolingThreshold = value;
        return true;
    }

    return false;
}

bool Globals::setExpansionTemp(double value)
{
    if (expansionTemp == value)
        return true;

    auto& db = DbManager::instance();
    if (db.updateGlobal("expansionTemp", QString::number(value))) {
        expansionTemp = value;
        return true;
    }

    return false;
}


Globals::Variables Globals::getVariables()
{
    return {
        serialDataTime, stateMachineTick
    };
}
