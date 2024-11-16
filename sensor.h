#ifndef SENSOR_H
#define SENSOR_H

#include <QMap>
#include <QString>

struct SensorValues {
    double temp;
    double tempK;
    double pressure;
};

class Sensor
{
public:
    Sensor(QString name, QString pinName, double minValue, double maxValue);

    void send(double value);
    void setValue(uint newPinValue);
    void setValue(QString newPinValue);

    static SensorValues getValues();
    static void parseSerialData(QString data);

    QString name, pinName;
    double minValue, maxValue;
    double value;
    uint pinValue;

    static qint64 lastDataTime;
    static QList<Sensor> sensors;
    static QMap<QString, Sensor *> mapName;
    static QMap<QString, Sensor *> mapPinName;
    bool static updateSensor(QString name, double minValue, double maxValue);
};

#endif // SENSOR_H
