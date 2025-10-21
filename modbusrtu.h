#ifndef MODBUSRTU_H
#define MODBUSRTU_H

#include <QObject>
#include <QtSerialBus/qmodbusrtuserialclient.h>
#include <QTimer>
#include <QDebug>
#include <QSet>
#include <QMutex>
#include <QDateTime>
#include <QSerialPort>  // Add this include

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
    bool isConnected() const { return modbusDevice && modbusDevice->state() == QModbusDevice::ConnectedState; }

private slots:
    void onReadReady();
    void onDigitalInputReady();
    void onErrorOccurred(QModbusDevice::Error error);
    void onStateChanged(QModbusDevice::State state);

private:
    QModbusRtuSerialClient *modbusDevice;
    QTimer readTimer;
    QTimer retryTimer;
    QTimer *cleanupTimer;
    QSet<QModbusReply*> pendingReplies;
    QMutex pendingRepliesMutex;

    // Connection parameters
    QString portName = "COM7";
    int baudRate = QSerialPort::Baud9600;
    int parity = QSerialPort::OddParity;
    int dataBits = QSerialPort::Data8;
    int stopBits = QSerialPort::OneStop;

    static constexpr int WAIT_TIME_MS = 2000;
    static constexpr int READ_INTERVAL_MS = 1000;
    static constexpr int MAX_PENDING_REQUESTS = 10;
    static constexpr int REQUEST_TIMEOUT_MS = 3000;
    static constexpr int REQUEST_DELAY_MS = 50; // Delay between sequential requests

    void attemptReconnect();
    void configureConnectionParameters();
    void cleanupPendingReplies();
    void cleanupStaleReplies();
    bool isRequestPending(quint8 slaveAddress, quint16 startAddr);
    void setupReplyConnections(QModbusReply *reply, const QString &type, quint8 slaveAddress, quint16 startAddr);
    void processSequentialReads();
    void sendSingleReadRequest(quint8 slaveAddress, quint16 startAddr, quint16 count, const QString &type);
};

#endif // MODBUSRTU_H
