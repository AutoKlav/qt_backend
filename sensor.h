#ifndef SENSOR_H
#define SENSOR_H

#include <QMap>
#include <QString>

struct SensorValues {
    // Analog values
    double temp;
    double expansionTemp;
    double heaterTemp;
    double tankTemp;
    double tempK;
    double tankWaterLevel;
    double pressure;
    double steamPressure;

    // Digital input values
    double doorClosed;
    double burnerFault;
    double waterShortage;
};
   
struct SensorRelayValues{
    unsigned int fillTankWithWater;
    unsigned int cooling;
    unsigned int tankHeating;
    unsigned int coolingHelper;
    unsigned int autoklavFill;
    unsigned int waterDrain;
    unsigned int heating;
    unsigned int pump;
    unsigned int electricHeating;
    unsigned int increasePressure;
    unsigned int extensionCooling;
    unsigned int alarmSignal;    
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
