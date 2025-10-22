#ifndef MODBUSRTU_H
#define MODBUSRTU_H

#include <QObject>
#include <QtSerialBus/qmodbusrtuserialclient.h>
#include <QTimer>
#include <QQueue>
#include <QDebug>
#include <functional>

struct ModbusRequest {
    quint8 slaveAddress;
    QModbusDataUnit::RegisterType registerType;
    quint16 startAddress;
    quint16 count;
    std::function<void(const QModbusDataUnit&, quint8)> callback;
    QString description; // For debugging
};

class ModbusRTU : public QObject
{
    Q_OBJECT
public:
    explicit ModbusRTU(QObject *parent = nullptr);
    static ModbusRTU &instance();
    static qint64 lastDataTime;

    void connectToDevice();
    void disconnectDevice();

    // Public API - these now queue requests
    void readHoldingRegisters(quint8 slaveAddress, quint16 startAddr, quint16 count, const QString &description = "");
    void readDiscreteRegisters(quint8 slaveAddress, quint16 startAddr, quint16 count, const QString &description = "");
    void writeSingleCoil(quint8 slaveAddress, quint16 coilAddress, bool value, const QString &description = "");

    bool isConnected() const { return modbusDevice && modbusDevice->state() == QModbusDevice::ConnectedState; }

private slots:
    void onReadReady();
    void onDigitalInputReady();
    void onErrorOccurred(QModbusDevice::Error error);
    void onStateChanged(QModbusDevice::State state);
    void processNextRequest();

private:
    QModbusRtuSerialClient *modbusDevice;
    QTimer readTimer;
    QTimer retryTimer;
    QTimer requestTimer;

    QQueue<ModbusRequest> requestQueue;
    bool isProcessingRequest = false;
    int retryCount = 0;

    static constexpr int WAIT_TIME_MS = 2000;
    static constexpr int READ_INTERVAL_MS = 1000;
    static constexpr int REQUEST_DELAY_MS = 50; // Delay between requests
    static constexpr int MAX_RETRIES = 5;

    void attemptReconnect();
    void configureConnectionParameters();
    void queueRequest(const ModbusRequest &request);
    void startSequentialReading();

    // Helper methods for specific sensor types
    void handleTemperatureReading(const QModbusDataUnit &unit, quint8 slaveAddress);
    void handlePressureReading(const QModbusDataUnit &unit, quint8 slaveAddress);
    void handleLevelReading(const QModbusDataUnit &unit, quint8 slaveAddress);
    void handleDigitalInputReading(const QModbusDataUnit &unit, quint8 slaveAddress, quint16 startAddress);
};

#endif // MODBUSRTU_H