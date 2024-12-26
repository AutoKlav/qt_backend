#include "sensor.h"

#include <QDateTime>

#include "globals.h"
#include "globalerrors.h"
#include "logger.h"
#include "dbmanager.h"
#include "serial.h"
#include "constants.h"

qint64 Sensor::lastDataTime = 0;
QList<Sensor> Sensor::sensors = QList<Sensor>();
QMap<QString, Sensor *> Sensor::mapName = QMap<QString, Sensor *>();

Sensor::Sensor(QString name, double minValue, double maxValue)
    : name{name}, minValue{minValue}, maxValue{maxValue}
{

}

void Sensor::send(double newValue)
{
    value = newValue; // Update the internal value
    uint pinValue = newValue;

    auto data = QString("%1=%2").arg(name).arg(pinValue);

    auto &serial = Serial::instance();
    serial.sendData(data);
}

void Sensor::setValue(uint newPinValue)
{    
    pinValue = newPinValue;

    // Determine the scaling factor based on the maxValue, analog Values will have maxValues above 1,
    // in order to scale them properly scaling factor should be 1023
    double scalingFactor = maxValue > 1 ? 1023.0 : 1.0;

    // Calculate the new value based on the scaling factor and the min/max range
    value = (newPinValue / scalingFactor) * (maxValue - minValue) + minValue;
}

void Sensor::setValue(QString newPinValue)
{
    setValue(newPinValue.toUInt());
}

void Sensor::checkIfDataIsOld()
{
    if (lastDataTime && QDateTime::currentMSecsSinceEpoch() - lastDataTime > Globals::serialDataTime)
        GlobalErrors::setError(GlobalErrors::OldDataError);
    else
        GlobalErrors::removeError(GlobalErrors::OldDataError);
}

/**
 * @brief Representing the sensor values mapped to virtual values. 
 */
SensorValues Sensor::getValues()
{
    checkIfDataIsOld();

    SensorValues values;

    values.temp = mapName[CONSTANTS::TEMP]->value;
    values.tempK = mapName[CONSTANTS::TEMP_K]->value;
    values.pressure = mapName[CONSTANTS::PRESSURE]->value;
    values.steamPressure = mapName[CONSTANTS::STEAM_PRESSURE]->value;
    values.waterLevel = mapName[CONSTANTS::WATER_LEVEL]->value;

    values.doorClosed = mapName[CONSTANTS::DOOR_CLOSED]->value;
    values.burnerFault = mapName[CONSTANTS::BURNER_FAULT]->value;
    values.waterShortage = mapName[CONSTANTS::WATER_SHORTAGE]->value;

    return values;
}

/** 
 * @brief Represents the raw values obtained from the sensor.
 */
SensorValues Sensor::getPinValues()
{
    checkIfDataIsOld();

    SensorValues values;

    values.temp = mapName[CONSTANTS::TEMP]->value;
    values.tempK = mapName[CONSTANTS::TEMP_K]->pinValue;
    values.pressure = mapName[CONSTANTS::PRESSURE]->pinValue;

    values.steamPressure = mapName[CONSTANTS::STEAM_PRESSURE]->pinValue;
    values.waterLevel = mapName[CONSTANTS::WATER_LEVEL]->pinValue;

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

    relayValues.heating = mapName[CONSTANTS::HEATING]->value;
    relayValues.pump = mapName[CONSTANTS::PUMP]->value;
    relayValues.cooling = mapName[CONSTANTS::COOLING]->value;

    return relayValues;
}

/**
 * @brief Parses the serial data received from the sensor and updates the corresponding sensor values.  
 * @param data The serial data received from the sensor.
 */
void Sensor::parseSerialData(QString data)
{
    // Parse data
    // Split data by sensors
    QList<QString> sensors = data.split(';');
    // Iterate over sensors
    for (const QString &sensor : sensors)
    {
        // Split sensor by name and value
        QList<QString> sensorData = sensor.split('=');
        // If sensor data is not valid continue
        if (sensorData.size() != 2)
            continue;

        // Get sensor name
        QString sensorName = sensorData[0];
        // Get sensor value
        QString sensorValue = sensorData[1];

        // Update sensor value if sensor exists
        if (mapName.contains(sensorName)) {
            mapName[sensorName]->setValue(sensorValue);
        } else {
            Logger::crit(QString("Sensor '%1' not found in database.").arg(sensorName));
            GlobalErrors::setError(GlobalErrors::DbError);
        }

        lastDataTime = QDateTime::currentMSecsSinceEpoch();
    }
}

bool Sensor::updateSensor(QString name, double minValue, double maxValue)
{
    auto isSensorUpdated = DbManager::instance().updateSensor(name, minValue, maxValue);
    return isSensorUpdated;
}

