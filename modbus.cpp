#include "modbus.h"
#include "sensor.h"
#include "logger.h"
#include "globalerrors.h"
#include <qvariant.h>

qint64 Modbus::lastDataTime = 0;

Modbus::Modbus(QObject *parent)
    : QObject{parent}, modbusClient(new QModbusTcpClient(this))
{
    connect(modbusClient, &QModbusClient::errorOccurred, this, &Modbus::onErrorOccurred);
    connect(modbusClient, &QModbusClient::stateChanged, this, &Modbus::onStateChanged);

    retryTimer.setInterval(WAIT_TIME_MS);
    connect(&retryTimer, &QTimer::timeout, this, &Modbus::attemptReconnect);
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

    attemptReconnect(); // Start initial connection
}


void Modbus::readInputRegisters()
{
    if (!modbusClient || modbusClient->state() != QModbusDevice::ConnectedState) {
        Logger::crit("Modbus client not connected. Cannot read input registers.");
        return;
    }

    QModbusDataUnit readUnit(QModbusDataUnit::InputRegisters, 0, 11);

    if (auto *reply = modbusClient->sendReadRequest(readUnit, 1)) {
        connect(reply, &QModbusReply::finished, this, [this, reply]() {
            if (reply->error() == QModbusDevice::NoError) {
                const QModbusDataUnit unit = reply->result();

                for (uint i = 0; i < unit.valueCount(); i++) {
                    // Update sensor value if sensor exists
                    if (Sensor::mapAnalogSensor.contains(i)) {
                        Logger::info(QString("Read i:%1  value:%2").arg(i).arg(unit.value(i)));
                        Sensor::mapAnalogSensor[i]->setValue(unit.value(i));
                    } else {
                        Logger::crit(QString("Sensor '%1' not found in AnalogSensor database.").arg(i));
                        GlobalErrors::setError(GlobalErrors::DbError);
                    }

                    lastDataTime = QDateTime::currentMSecsSinceEpoch();
                }
            } else {
                Logger::crit(QString("Read error: %1").arg(reply->errorString()));
            }
            reply->deleteLater();

            // Add a delay before the next read request
            QTimer::singleShot(READ_INTERVAL_MS, this, &Modbus::readInputRegisters);
        });
    } else {
        Logger::crit(QString("Read request failed: %1").arg(modbusClient->errorString()));
    }
}

void Modbus::writeMultipleCoils()
{
    if (!modbusClient || modbusClient->state() != QModbusDevice::ConnectedState) {        
        Logger::crit("Modbus client not connected. Cannot write coils.");
        return;
    }

    QVector<quint16> coilValues = {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1};
    QModbusDataUnit writeUnit(QModbusDataUnit::Coils, 0, coilValues.size());

    for (int i = 0; i < coilValues.size(); ++i) {
        writeUnit.setValue(i, coilValues[i]);
    }

    if (auto *reply = modbusClient->sendWriteRequest(writeUnit, 1)) {
        connect(reply, &QModbusReply::finished, this, [reply]() {
            if (reply->error() == QModbusDevice::NoError) {                
                Logger::info("Coils written successfully!");
            } else {
                Logger::crit(QString("Write error: %1").arg(reply->errorString()));
            }
            reply->deleteLater();
        });
    } else {
        Logger::crit(QString("Write request failed: %1").arg(modbusClient->errorString()));
    }
}

void Modbus::writeSingleCoil(int coilAddress, bool value)
{
    if (!modbusClient || modbusClient->state() != QModbusDevice::ConnectedState) {        
        Logger::crit("Modbus client not connected. Cannot write to coil.");
        return;
    }

    QModbusDataUnit writeUnit(QModbusDataUnit::Coils, coilAddress, 1);
    writeUnit.setValue(0, value);

    if (auto *reply = modbusClient->sendWriteRequest(writeUnit, 1)) {
        connect(reply, &QModbusReply::finished, this, [reply]() {
            if (reply->error() != QModbusDevice::NoError) {
                Logger::crit(QString("Write error: %1").arg(reply->errorString()));                
            } 

            reply->deleteLater();
        });
    } else {
        Logger::crit(QString("Write request failed: %1").arg(modbusClient->errorString()));
    }
}

void Modbus::onErrorOccurred(QModbusDevice::Error error)
{
    if (error != QModbusDevice::NoError) {
        Logger::crit(QString("Modbus error: %1").arg(modbusClient->errorString()));
        if (modbusClient->state() != QModbusDevice::ConnectedState) {
            retryTimer.start();
        }
    }
}

void Modbus::onStateChanged(QModbusDevice::State state)
{
    if (state == QModbusDevice::ConnectedState) {
        Logger::info("Modbus client successfully connected.");
        retryTimer.stop();

        // Start reading input registers after connection
        QTimer::singleShot(1000, this, &Modbus::readInputRegisters);
    } else if (state == QModbusDevice::UnconnectedState) {
        Logger::info("Modbus client disconnected. Will attempt reconnection.");
        retryTimer.start();
    }
}


void Modbus::attemptReconnect()
{
    if (modbusClient->state() == QModbusDevice::ConnectedState) {
        return;
    }

    Logger::info("Attempting to reconnect to the Modbus server...");

    // Ensure a fresh connection by properly disconnecting first
    if (modbusClient->state() != QModbusDevice::UnconnectedState) {
        modbusClient->disconnectDevice();
    }

    QTimer::singleShot(500, this, [this]() {
        if (!modbusClient->connectDevice()) {
            Logger::crit(QString("Reconnection attempt failed: %1").arg(modbusClient->errorString()));
            retryTimer.start();
        }
    });
}
