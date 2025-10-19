#ifndef MODBUSRTU_H
#define MODBUSRTU_H

#include <QObject>
#include <QtSerialBus/qmodbusrtuserialclient.h>
#include <QTimer>
#include <QDebug>
#include <QSet>

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
    void onDigitalInputReady();
    void onErrorOccurred(QModbusDevice::Error error);
    void onStateChanged(QModbusDevice::State state);

private:
    QModbusRtuSerialClient *modbusDevice;
    QTimer readTimer;
    QTimer retryTimer;
    QSet<QModbusReply*> pendingReplies;

    static constexpr int WAIT_TIME_MS = 2000;
    static constexpr int READ_INTERVAL_MS = 1000;
    static constexpr int MAX_PENDING_REQUESTS = 10;
    static constexpr int REQUEST_TIMEOUT_MS = 3000;

    void attemptReconnect();
    void configureConnectionParameters();
    void cleanupPendingReplies();
    void cleanupStaleReplies();
    bool isRequestPending(quint8 slaveAddress, quint16 startAddr);
    void setupReplyConnections(QModbusReply *reply, const QString &type);
};

#endif // MODBUSRTU_H
