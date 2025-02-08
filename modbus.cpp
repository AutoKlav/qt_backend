#include "modbus.h"
#include <qvariant.h>

Modbus::Modbus(QObject *parent)
    : QObject{parent}, modbusClient(new QModbusTcpClient(this))
{
    connect(modbusClient, &QModbusClient::errorOccurred, this, &Modbus::onErrorOccurred);
    connect(modbusClient, &QModbusClient::stateChanged, this, &Modbus::onStateChanged);

    // Initialize the retry timer with a WAIT_TIME_MS-second interval
    retryTimer.setInterval(WAIT_TIME_MS);
    connect(&retryTimer, &QTimer::timeout, this, [this]() {
        qDebug() << "Attempting to reconnect to the Modbus server...";
        if (modbusClient->connectDevice()) {
            qDebug() << "Reconnected successfully.";
            retryTimer.stop();
        } else {
            qCritical() << "Failed to reconnect, error:" << modbusClient->errorString();
        }
    });
}

Modbus &Modbus::instance()
{
    static Modbus _instance;
    return _instance;
}

Modbus::~Modbus() {}

void Modbus::connectToServer(const QString &ip, int port)
{
    modbusClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter, ip);
    modbusClient->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);

    if (!modbusClient->connectDevice()) {
        qCritical() << "Connection failed:" << modbusClient->errorString();
        retryTimer.start(); // Start retry timer if the initial connection fails
    }
}

void Modbus::readInputRegisters()
{
    if (!modbusClient) return;

    QModbusDataUnit readUnit(QModbusDataUnit::InputRegisters, 0, 12);

    if (auto *reply = modbusClient->sendReadRequest(readUnit, 1)) {
        connect(reply, &QModbusReply::finished, this, [reply]() {
            if (reply->error() == QModbusDevice::NoError) {
                const QModbusDataUnit unit = reply->result();
                qDebug() << "Input Registers:";
                for (uint i = 0; i < unit.valueCount(); i++) {
                    qDebug() << "Register" << i << ":" << unit.value(i);
                }
            } else {
                qCritical() << "Read error:" << reply->errorString();
            }
            reply->deleteLater();
        });
    } else {
        qCritical() << "Read request failed:" << modbusClient->errorString();
    }
}

void Modbus::writeMultipleCoils()
{
    if (!modbusClient) return;

    QVector<quint16> coilValues = {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1};
    QModbusDataUnit writeUnit(QModbusDataUnit::Coils, 0, coilValues.size());

    for (int i = 0; i < coilValues.size(); ++i) {
        writeUnit.setValue(i, coilValues[i]);
    }

    if (auto *reply = modbusClient->sendWriteRequest(writeUnit, 1)) {
        connect(reply, &QModbusReply::finished, this, [reply]() {
            if (reply->error() == QModbusDevice::NoError) {
                qDebug() << "Coils written successfully!";
            } else {
                qCritical() << "Write error:" << reply->errorString();
            }
            reply->deleteLater();
        });
    } else {
        qCritical() << "Write request failed:" << modbusClient->errorString();
    }
}

void Modbus::onErrorOccurred(QModbusDevice::Error error)
{
    if (error != QModbusDevice::NoError) {
        qCritical() << "Modbus error:" << modbusClient->errorString();
        if (modbusClient->state() != QModbusDevice::ConnectedState) {
            modbusClient->disconnectDevice();
            retryTimer.start(); // Start retry timer if disconnected due to an error
        }
    }
}

void Modbus::onStateChanged(QModbusDevice::State state)
{
    if (state == QModbusDevice::ConnectedState) {
        qDebug() << "Modbus client connected.";
    } else if (state == QModbusDevice::UnconnectedState) {
        qDebug() << "Modbus client disconnected.";
        retryTimer.start(); // Start retry timer if disconnected
    }
}
