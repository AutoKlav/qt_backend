#ifndef SENSOR_H
#define SENSOR_H

#include <QMap>
#include <QString>

struct SensorValues {
    double temp;
    double tempK;
    double pressure;
};

struct SensorRelayValues{
    unsigned short waterFill;
    unsigned short heating;
    unsigned short bypass;
    unsigned short pump;
    unsigned short inPressure;
    unsigned short cooling;
};

class Sensor
{
public:
    Sensor(QString name, double minValue, double maxValue);

    void send(double value);
    void setValue(uint newPinValue);
    void setValue(QString newPinValue);

    static SensorValues getValues();
    static SensorValues getPinValues();
    static SensorRelayValues getRelayValues();
    static void parseSerialData(QString data);
    static void checkIfDataIsOld();
    
    QString name;
    double minValue, maxValue;
    double value;
    uint pinValue;

    static qint64 lastDataTime;
    static QList<Sensor> sensors;
    static QMap<QString, Sensor *> mapName;    
    static bool updateSensor(QString name, double minValue, double maxValue);
};

#endif // SENSOR_H
