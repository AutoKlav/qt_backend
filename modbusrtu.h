#ifndef MODBUSRTU_H
#define MODBUSRTU_H

#include <QObject>
#include <QtSerialBus/qmodbusrtuserialclient.h>  // Updated header
#include <QTimer>
#include <QDebug>

class ModbusRTU : public QObject
{
    Q_OBJECT
public:
    explicit ModbusRTU(QObject *parent = nullptr);
    static ModbusRTU &instance();
    static qint64 lastDataTime;

    void connectToDevice();
    void readHoldingRegisters(quint8 slaveAddress, quint16 startAddr, quint16 count);
    void readDiscreteRegisters(quint8 slaveAddress, quint16 startAddr, quint16 count);
    void writeSingleCoil(quint8 slaveAddress, quint16 coilAddress, bool value);

private slots:
    void onReadReady();
    void onErrorOccurred(QModbusDevice::Error error);
    void onStateChanged(QModbusDevice::State state);

private:
    QModbusRtuSerialClient *modbusDevice;
    QTimer readTimer;
    QTimer retryTimer;

    static constexpr int WAIT_TIME_MS = 2000; /**< The wait time in milliseconds for reconnection attempts. */
    static constexpr int READ_INTERVAL_MS = 1000; /**< The interval in milliseconds for reading sensor data. */

    void attemptReconnect();  // Add reconnect method
    void configureConnectionParameters();  // Helper to set connection parameters
};

#endif // MODBUSRTU_H
