#include "sensor.h"

#include <QDateTime>

#include "globalerrors.h"
#include "globals.h"
#include "logger.h"
#include "dbmanager.h"
#include "constants.h"
#include "modbus.h"

qint64 Sensor::lastDataTime = 0;
QList<Sensor> Sensor::analogSensors = QList<Sensor>();
QList<Sensor> Sensor::digitalSensors = QList<Sensor>();

QMap<ushort, Sensor *> Sensor::mapAnalogSensor = QMap<ushort, Sensor *>();
QMap<ushort, Sensor *> Sensor::mapDigitalSensor = QMap<ushort, Sensor *>();

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
    if (mapDigitalSensor.contains(id)) {
        Sensor::mapDigitalSensor[id]->send(value);
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

void Sensor::setValue(ushort newPinValue)
{    
    pinValue = newPinValue;

    // Determine the scaling factor based on the maxValue, analog Values will have maxValues above 1,
    // in order to scale them properly scaling factor should be 1023
    double scalingFactor = maxValue >= 1 ? 1023.0 : 1.0;

    // Calculate the new value based on the scaling factor and the min/max range
    value = (newPinValue / scalingFactor) * (maxValue - minValue) + minValue;
}

/**
 * @brief Representing the sensor values mapped to virtual values. 
 */
SensorValues Sensor::getValues()
{
    checkIfDataIsOld();

    SensorValues values;
    
    values.temp = mapAnalogSensor[CONSTANTS::TEMP]->value;
    values.expansionTemp = mapAnalogSensor[CONSTANTS::EXPANSION_TEMP]->value;
    values.heaterTemp = mapAnalogSensor[CONSTANTS::HEATER_TEMP]->value;
    values.tankTemp = mapAnalogSensor[CONSTANTS::TANK_TEMP]->value;
    values.tempK = mapAnalogSensor[CONSTANTS::TEMP_K]->value;
    values.tankWaterLevel = mapAnalogSensor[CONSTANTS::TANK_WATER_LEVEL]->value;
    values.pressure = mapAnalogSensor[CONSTANTS::PRESSURE]->value;
    values.steamPressure = mapAnalogSensor[CONSTANTS::STEAM_PRESSURE]->value;

    values.doorClosed = mapAnalogSensor[CONSTANTS::DOOR_CLOSED]->value;
    values.burnerFault = mapAnalogSensor[CONSTANTS::BURNER_FAULT]->value;
    values.waterShortage = mapAnalogSensor[CONSTANTS::WATER_SHORTAGE]->value;
    
    return values;
}

/** 
 * @brief Represents the raw values obtained from the sensor.
 */
SensorValues Sensor::getPinValues()
{
    checkIfDataIsOld();

    SensorValues values;

    values.temp = mapAnalogSensor[CONSTANTS::TEMP]->pinValue;
    values.expansionTemp = mapAnalogSensor[CONSTANTS::EXPANSION_TEMP]->pinValue;
    values.heaterTemp = mapAnalogSensor[CONSTANTS::HEATER_TEMP]->pinValue;
    values.tankTemp = mapAnalogSensor[CONSTANTS::TANK_TEMP]->pinValue;
    values.tempK = mapAnalogSensor[CONSTANTS::TEMP_K]->pinValue;
    values.tankWaterLevel = mapAnalogSensor[CONSTANTS::TANK_WATER_LEVEL]->pinValue;
    values.pressure = mapAnalogSensor[CONSTANTS::PRESSURE]->pinValue;
    values.steamPressure = mapAnalogSensor[CONSTANTS::STEAM_PRESSURE]->pinValue;

    values.doorClosed = mapAnalogSensor[CONSTANTS::DOOR_CLOSED]->pinValue;
    values.burnerFault = mapAnalogSensor[CONSTANTS::BURNER_FAULT]->pinValue;
    values.waterShortage = mapAnalogSensor[CONSTANTS::WATER_SHORTAGE]->pinValue;

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
    
    relayValues.fillTankWithWater = mapDigitalSensor[CONSTANTS::FILL_TANK_WITH_WATER]->value;
    relayValues.cooling = mapDigitalSensor[CONSTANTS::COOLING]->value;
    relayValues.tankHeating = mapDigitalSensor[CONSTANTS::TANK_HEATING]->value;
    relayValues.coolingHelper = mapDigitalSensor[CONSTANTS::COOLING_HELPER]->value;
    relayValues.autoklavFill = mapDigitalSensor[CONSTANTS::AUTOKLAV_FILL]->value;
    relayValues.waterDrain = mapDigitalSensor[CONSTANTS::WATER_DRAIN]->value;
    relayValues.heating = mapDigitalSensor[CONSTANTS::STEAM_HEATING]->value;
    relayValues.pump = mapDigitalSensor[CONSTANTS::PUMP]->value;
    relayValues.electricHeating = mapDigitalSensor[CONSTANTS::ELECTRIC_HEATING]->value;
    relayValues.increasePressure = mapDigitalSensor[CONSTANTS::INCREASE_PRESSURE]->value;
    relayValues.extensionCooling = mapDigitalSensor[CONSTANTS::EXTENSION_COOLING]->value;
    relayValues.alarmSignal = mapDigitalSensor[CONSTANTS::ALARM_SIGNAL]->value;
    
    return relayValues;
}

bool Sensor::updateAnalogSensor(ushort id, double minValue, double maxValue)
{
    if (!DbManager::instance().updateAnalogSensor(id, minValue, maxValue))
        return false;

    if (mapAnalogSensor.contains(id))
        return false;

    mapAnalogSensor[id]->minValue = minValue;
    mapAnalogSensor[id]->maxValue = maxValue;

    return true;
}

void Sensor::checkIfDataIsOld()
{
    if (lastDataTime && QDateTime::currentMSecsSinceEpoch() - lastDataTime > Globals::serialDataOldTime)
        GlobalErrors::setError(GlobalErrors::OldDataError);
    else
        GlobalErrors::removeError(GlobalErrors::OldDataError);
}
