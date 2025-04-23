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
    Sensor(ushort id, double minValue, double maxValue);
    Sensor(ushort id);

    void send(double newValue);
    void sendIfNew(double newValue);
    void setValue(ushort newPinValue);
    
    static SensorValues getValues();
    static SensorValues getPinValues();
    static SensorRelayValues getRelayValues();
    static bool setRelayState(ushort id, ushort value);
    static void parseModbusData(QString data);
    static void checkIfDataIsOld();
    
    ushort id; // position of the I/O port in the PLC
    double minValue, maxValue;
    double value;  // parsed value
    ushort pinValue; // Raw data used for calibration

    static qint64 lastDataTime;
    static QList<Sensor> inputPins;
    static QList<Sensor> outputPins;
    static QMap<ushort, Sensor *> mapInputPin;
    static QMap<ushort, Sensor *> mapOutputPin;
    static bool updateInputPin(ushort id, double minValue, double maxValue);
};

#endif // SENSOR_H
