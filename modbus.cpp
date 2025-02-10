#include "modbus.h"
#include "sensor.h"
#include "logger.h"
#include "globalerrors.h"
#include "globals.h"
#include <qvariant.h>

qint64 Modbus::lastDataTime = 0; // Define the static member outside the class

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

    if (!modbusClient->connectDevice()) {
        qCritical() << "Initial connection failed:" << modbusClient->errorString();
        retryTimer.start();
    }
}

void Modbus::readInputRegisters()
{
    if (!modbusClient || modbusClient->state() != QModbusDevice::ConnectedState) {
        qCritical() << "Modbus client not connected. Cannot read input registers.";
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
                        Sensor::mapAnalogSensor[i]->setValue(unit.value(i));
                    } else {
                        Logger::crit(QString("Sensor '%1' not found in AnalogSensor database.").arg(i));
                        GlobalErrors::setError(GlobalErrors::DbError);
                    }

                    lastDataTime = QDateTime::currentMSecsSinceEpoch();
                }
            } else {
                qCritical() << "Read error:" << reply->errorString();
            }
            reply->deleteLater();

            // Add a delay before the next read request
            QTimer::singleShot(READ_INTERVAL_MS, this, &Modbus::readInputRegisters); // 1-second delay
        });
    } else {
        qCritical() << "Read request failed:" << modbusClient->errorString();
    }
}

void Modbus::writeMultipleCoils()
{
    if (!modbusClient || modbusClient->state() != QModbusDevice::ConnectedState) {
        qCritical() << "Modbus client not connected. Cannot write coils.";
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
            retryTimer.start();
        }
    }
}

void Modbus::onStateChanged(QModbusDevice::State state)
{
    switch (state) {
    case QModbusDevice::ConnectedState:
        qDebug() << "Modbus client connected.";
        retryTimer.stop();
        break;
    case QModbusDevice::UnconnectedState:
        qDebug() << "Modbus client disconnected. Starting reconnect timer.";
        retryTimer.start();
        break;
    default:
        break;
    }
}

void Modbus::attemptReconnect()
{
    qDebug() << "Attempting to reconnect to the Modbus server...";
    if (modbusClient->connectDevice()) {
        qDebug() << "Reconnected successfully.";
    } else {
        qCritical() << "Reconnection attempt failed: " << modbusClient->errorString();
    }
}