#include "sensor.h"

#include <QDateTime>

#include "globals.h"
#include "globalerrors.h"
#include "logger.h"
#include "serial.h"
#include "dbmanager.h"

qint64 Sensor::lastDataTime = 0;
QList<Sensor> Sensor::sensors = QList<Sensor>();
QMap<QString, Sensor *> Sensor::mapName = QMap<QString, Sensor *>();
QMap<QString, Sensor *> Sensor::mapPinName = QMap<QString, Sensor *>();

Sensor::Sensor(QString name, QString pinName, double minValue, double maxValue)
    : name{name}, pinName{pinName}, minValue{minValue}, maxValue{maxValue}
{

}

void Sensor::send(double newValue)
{
    value = newValue; // Update the internal value
    uint pinValue = (newValue - minValue) / (maxValue - minValue) * 1023;

    auto data = QString("%1=%2").arg(pinName).arg(pinValue);

    auto &serial = Serial::instance();
    serial.sendData(data);
}

void Sensor::setValue(uint newPinValue)
{    
    pinValue = newPinValue;
    value = (newPinValue / 1023.0) * (maxValue - minValue) + minValue;
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

    values.temp = mapName["temp"]->value;
    values.tempK = mapName["tempK"]->value;
    values.pressure = mapName["pressure"]->value;

    return values;
}

/** 
 * @brief Represents the raw values obtained from the sensor.
 */
SensorValues Sensor::getPinValues()
{
    checkIfDataIsOld();

    SensorValues values;

    values.temp = mapName["temp"]->pinValue;
    values.tempK = mapName["tempK"]->pinValue;
    values.pressure = mapName["pressure"]->pinValue;

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

    relayValues.waterFill = mapName["waterFill"]->value;
    relayValues.heating = mapName["heating"]->value;
    relayValues.bypass = mapName["bypass"]->value;
    relayValues.pump = mapName["pump"]->value;
    relayValues.inPressure = mapName["inPressure"]->value;
    relayValues.cooling = mapName["cooling"]->value;    

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
        if (mapPinName.contains(sensorName)) {
            mapPinName[sensorName]->setValue(sensorValue);
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

