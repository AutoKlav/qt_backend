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

void Sensor::send(double value)
{
    uint pinValue = (value - minValue) / (maxValue - minValue) * 1023;

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

SensorValues Sensor::getValues()
{
    // Check if data is old
    if (lastDataTime && QDateTime::currentMSecsSinceEpoch() - lastDataTime > Globals::serialDataTime)
        GlobalErrors::setError(GlobalErrors::OldDataError);
    else
        GlobalErrors::removeError(GlobalErrors::OldDataError);

    return {
        .temp = mapName["temp"]->value,
        .tempK = mapName["tempK"]->value,
        .pressure = mapName["pressure"]->value,
    };
}

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

bool updateSensor(QString name, double minValue, double maxValue)
{
    auto isSensorUpdated = DbManager::instance().updateSensor(name, minValue, maxValue);
    return isSensorUpdated;
}

