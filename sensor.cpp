#include "sensor.h"

#include <QDateTime>

#include "globalerrors.h"
#include "globals.h"
#include "logger.h"
#include "dbmanager.h"
#include "constants.h"
#include "modbus.h"

qint64 Sensor::lastDataTime = 0;
QList<Sensor> Sensor::inputPins = QList<Sensor>();
QList<Sensor> Sensor::outputPins = QList<Sensor>();

QMap<ushort, Sensor *> Sensor::mapInputPin = QMap<ushort, Sensor *>();
QMap<ushort, Sensor *> Sensor::mapOutputPin = QMap<ushort, Sensor *>();

Sensor::Sensor(ushort id, double minValue, double maxValue)
    : id{id}, minValue{minValue}, maxValue{maxValue}
{

}

Sensor::Sensor(ushort id)
    : id{id}
{

}

void Sensor::send(double newValue)
{
    value = newValue; // Update the internal value
    uint pinValue = newValue;

    Modbus::instance().writeSingleCoil(id, newValue);
}

bool Sensor::setRelayState(ushort id, ushort value)
{
    // Update sensor value if sensor exists
    if (mapOutputPin.contains(id)) {
        Sensor::mapOutputPin[id]->send(value);
    } else {
        Logger::crit(QString("Sensor '%1' cannot be set to '%2' since it is not found in database.").arg(id).arg(value));
        GlobalErrors::setError(GlobalErrors::DbError);
        return false;
    }

    return true;
}

void Sensor::sendIfNew(double newValue)
{
    if (newValue == value)
        return;

    send(newValue);
}

void Sensor::setValue(uint newPinValue)
{    
    pinValue = newPinValue;

    double scalingFactor;

    if(id < 7){
        // for 4-20mA range
        scalingFactor = (newPinValue - 4000.0) / (20000.0 - 4000.0);
    } else if ( id == 7) {
        // for 0-10v
        scalingFactor = (newPinValue - 0.0) / (10000.0 - 0.0);
    } else {
        // DI pins, should be between 0 and 1
        scalingFactor = newPinValue;
    }

    // Calculate the new value based on the scaling factor and the min/max range
    value = scalingFactor * (maxValue - minValue) + minValue;
}

/**
 * @brief Representing the sensor values mapped to virtual values. 
 */
SensorValues Sensor::getValues()
{
    checkIfDataIsOld();

    SensorValues values;
    
    values.temp = mapInputPin[CONSTANTS::TEMP]->value;
    values.expansionTemp = mapInputPin[CONSTANTS::EXPANSION_TEMP]->value;
    values.heaterTemp = mapInputPin[CONSTANTS::HEATER_TEMP]->value;
    values.tankTemp = mapInputPin[CONSTANTS::TANK_TEMP]->value;
    values.tempK = mapInputPin[CONSTANTS::TEMP_K]->value;
    values.tankWaterLevel = mapInputPin[CONSTANTS::TANK_WATER_LEVEL]->value;
    values.pressure = mapInputPin[CONSTANTS::PRESSURE]->value;
    values.steamPressure = mapInputPin[CONSTANTS::STEAM_PRESSURE]->value;

    values.doorClosed = mapInputPin[CONSTANTS::DOOR_CLOSED]->value;
    values.burnerFault = mapInputPin[CONSTANTS::BURNER_FAULT]->value;
    values.waterShortage = mapInputPin[CONSTANTS::WATER_SHORTAGE]->value;
    
    return values;
}

/** 
 * @brief Represents the raw values obtained from the sensor.
 */
SensorValues Sensor::getPinValues()
{
    checkIfDataIsOld();

    SensorValues values;

    values.temp = mapInputPin[CONSTANTS::TEMP]->pinValue;
    values.expansionTemp = mapInputPin[CONSTANTS::EXPANSION_TEMP]->pinValue;
    values.heaterTemp = mapInputPin[CONSTANTS::HEATER_TEMP]->pinValue;
    values.tankTemp = mapInputPin[CONSTANTS::TANK_TEMP]->pinValue;
    values.tempK = mapInputPin[CONSTANTS::TEMP_K]->pinValue;
    values.tankWaterLevel = mapInputPin[CONSTANTS::TANK_WATER_LEVEL]->pinValue;
    values.pressure = mapInputPin[CONSTANTS::PRESSURE]->pinValue;
    values.steamPressure = mapInputPin[CONSTANTS::STEAM_PRESSURE]->pinValue;

    values.doorClosed = mapInputPin[CONSTANTS::DOOR_CLOSED]->pinValue;
    values.burnerFault = mapInputPin[CONSTANTS::BURNER_FAULT]->pinValue;
    values.waterShortage = mapInputPin[CONSTANTS::WATER_SHORTAGE]->pinValue;

    return values;
}

/**
 * @brief Structure representing the relay values of a sensor. Check void Sensor::setValue(uint newPinValue)
    * for more information.
 */
SensorRelayValues Sensor::getRelayValues()
{
    checkIfDataIsOld();

    SensorRelayValues relayValues;
    
    relayValues.fillTankWithWater = mapOutputPin[CONSTANTS::FILL_TANK_WITH_WATER]->value;
    relayValues.cooling = mapOutputPin[CONSTANTS::COOLING]->value;
    relayValues.tankHeating = mapOutputPin[CONSTANTS::TANK_HEATING]->value;
    relayValues.coolingHelper = mapOutputPin[CONSTANTS::COOLING_HELPER]->value;
    relayValues.autoklavFill = mapOutputPin[CONSTANTS::AUTOKLAV_FILL]->value;
    relayValues.waterDrain = mapOutputPin[CONSTANTS::WATER_DRAIN]->value;
    relayValues.heating = mapOutputPin[CONSTANTS::STEAM_HEATING]->value;
    relayValues.pump = mapOutputPin[CONSTANTS::PUMP]->value;
    relayValues.electricHeating = mapOutputPin[CONSTANTS::ELECTRIC_HEATING]->value;
    relayValues.increasePressure = mapOutputPin[CONSTANTS::INCREASE_PRESSURE]->value;
    relayValues.extensionCooling = mapOutputPin[CONSTANTS::EXTENSION_COOLING]->value;
    relayValues.alarmSignal = mapOutputPin[CONSTANTS::ALARM_SIGNAL]->value;
    
    return relayValues;
}

bool Sensor::updateInputPin(ushort id, double minValue, double maxValue)
{
    if (!DbManager::instance().updateInputPin(id, minValue, maxValue))
        return false;

    if (mapInputPin.contains(id))
        return false;

    mapInputPin[id]->minValue = minValue;
    mapInputPin[id]->maxValue = maxValue;

    return true;
}

void Sensor::checkIfDataIsOld()
{
    if (lastDataTime && QDateTime::currentMSecsSinceEpoch() - lastDataTime > Globals::serialDataOldTime)
        GlobalErrors::setError(GlobalErrors::OldDataError);
    else
        GlobalErrors::removeError(GlobalErrors::OldDataError);
}
