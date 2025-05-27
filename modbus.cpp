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

    writeQueueTimer.setInterval(WRITE_DELAY_MS);
    connect(&writeQueueTimer, &QTimer::timeout, this, &Modbus::processWriteQueue);
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

void Modbus::readAllInputs()
{
    if (!modbusClient || modbusClient->state() != QModbusDevice::ConnectedState) {
        Logger::crit("Modbus client not connected. Cannot read inputs.");
        return;
    }

    // First read analog inputs
    readAnalogInputs();
}

void Modbus::readAnalogInputs()
{
    // Reading UINT16 values (40051-40051+n, function code 03)
    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, 0x32, 8);

    if (auto *reply = modbusClient->sendReadRequest(readUnit, 1)) {
        connect(reply, &QModbusReply::finished, this, [this, reply]() {
            if (reply->error() == QModbusDevice::NoError) {
                const QModbusDataUnit unit = reply->result();

                // Process analog inputs (AI0-AI7) -> mapInputPin[0-7]
                for (uint i = 0; i < unit.valueCount(); i++) {
                    quint16 rawValue = unit.value(i);                    

                    if (Sensor::mapInputPin.contains(i)) {
                        Logger::info(QString("AI%1 -> mapInputPin[%2] - Raw: %3")
                                         .arg(i).arg(i).arg(rawValue));
                        Sensor::mapInputPin[i]->setValue(rawValue);
                    } else {
                        Logger::crit(QString("Analog sensor at mapInputPin[%1] not found").arg(i));
                        GlobalErrors::setError(GlobalErrors::DbError);
                    }
                }

                // After analog inputs are read, read digital inputs
                readDigitalInputsSequential();

            } else {
                Logger::crit(QString("Analog input read error: %1").arg(reply->errorString()));
                GlobalErrors::setError(GlobalErrors::ModbusReadRegisterError);
                // Still try to read digital inputs even if analog failed
                readDigitalInputsSequential();
            }
            reply->deleteLater();
        });
    } else {
        Logger::crit(QString("Analog input read request failed: %1").arg(modbusClient->errorString()));
        // Try digital inputs anyway
        readDigitalInputsSequential();
    }
}

void Modbus::readDigitalInputsSequential()
{
    // Read 3 digital inputs (DI0, DI1, DI2) → Modbus addresses 0x0000, 0x0001, 0x0002
    QModbusDataUnit readUnit(QModbusDataUnit::DiscreteInputs, 0x0000, 3);

    if (auto *reply = modbusClient->sendReadRequest(readUnit, 1)) {
        connect(reply, &QModbusReply::finished, this, [this, reply]() {
            if (reply->error() == QModbusDevice::NoError) {
                const QModbusDataUnit unit = reply->result();

                // Process DI0 (mapInputPin[8]), DI1 (mapInputPin[9]), DI2 (mapInputPin[10])
                for (uint i = 0; i < unit.valueCount()-5; i++) { // With 8 DI, we read first 3
                    bool digitalState = unit.value(i) == 1;
                    float digitalValue = digitalState ? 1.0f : 0.0f;

                    uint mapIndex = i + 8; // Maps DI0 → 8, DI1 → 9, DI2 → 10

                    if (Sensor::mapInputPin.contains(mapIndex)) {
                        Logger::info(QString("DI%1 -> mapInputPin[%2] - State: %3, Value: %4")
                                         .arg(i).arg(mapIndex)
                                         .arg(digitalState ? "ON" : "OFF")
                                         .arg(digitalValue));
                        Sensor::mapInputPin[mapIndex]->setValue(digitalValue);
                    } else {
                        Logger::crit(QString("Digital sensor at mapInputPin[%1] not found").arg(mapIndex));
                        GlobalErrors::setError(GlobalErrors::DbError);
                    }
                }
            } else {
                Logger::crit(QString("Digital input read error: %1").arg(reply->errorString()));
                GlobalErrors::setError(GlobalErrors::ModbusReadRegisterError);
            }

            reply->deleteLater();
            GlobalErrors::removeError(GlobalErrors::ModbusReadRegisterError);
            lastDataTime = QDateTime::currentMSecsSinceEpoch();

            // Schedule next read cycle
            QTimer::singleShot(READ_INTERVAL_MS, this, &Modbus::readAllInputs);
        });
    } else {
        Logger::crit(QString("Digital input read request failed: %1").arg(modbusClient->errorString()));
        // Schedule next read cycle even if this failed
        QTimer::singleShot(READ_INTERVAL_MS, this, &Modbus::readAllInputs);
    }
}

void Modbus::writeSingleCoil(int coilAddress, bool value)
{
    if (!modbusClient || modbusClient->state() != QModbusDevice::ConnectedState) {
        Logger::crit("Modbus client not connected. Cannot write to coil.");
        return;
    }

    // Add to queue instead of writing immediately
    coilWriteQueue.enqueue(qMakePair(coilAddress, value));

    if (!writeQueueTimer.isActive() && !isWriting) {
        writeQueueTimer.start();
    }
}

void Modbus::processWriteQueue()
{
    if (isWriting || coilWriteQueue.isEmpty()) {
        if (coilWriteQueue.isEmpty()) {
            writeQueueTimer.stop();
        }
        return;
    }

    if (!modbusClient || modbusClient->state() != QModbusDevice::ConnectedState) {
        writeQueueTimer.stop();
        return;
    }

    isWriting = true;
    QPair<int, bool> operation = coilWriteQueue.dequeue();
    int coilAddress = operation.first;
    bool value = operation.second;

    QModbusDataUnit writeUnit(QModbusDataUnit::Coils, coilAddress, 1);
    writeUnit.setValue(0, value ? 0xFF00 : 0x0000);

    if (auto *reply = modbusClient->sendWriteRequest(writeUnit, 1)) {
        connect(reply, &QModbusReply::finished, this, [this, coilAddress, value, reply]() {
            isWriting = false;

            if (reply->error() != QModbusDevice::NoError) {
                Logger::crit(QString("Failed to write coil %1: %2")
                                 .arg(coilAddress)
                                 .arg(reply->errorString()));
                GlobalErrors::setError(GlobalErrors::ModbusWriteCoilError);
            } else {
                //Logger::info(QString("Successfully set DO%1 to %2")
                                 //.arg(coilAddress)
                                 //.arg(value ? "ON" : "OFF"));
            }

            reply->deleteLater();
            GlobalErrors::removeError(GlobalErrors::ModbusWriteCoilError);

            // Process next item immediately if available
            QTimer::singleShot(0, this, &Modbus::processWriteQueue);
        });
    } else {
        isWriting = false;
        Logger::crit(QString("Write request failed for DO%1: %2")
                         .arg(coilAddress)
                         .arg(modbusClient->errorString()));
        GlobalErrors::setError(GlobalErrors::ModbusWriteCoilError);

        // Retry after delay
        QTimer::singleShot(WRITE_DELAY_MS, this, &Modbus::processWriteQueue);
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
        GlobalErrors::removeError(GlobalErrors::ModbusError);
        retryTimer.stop();

        // Start reading input registers after connection
        QTimer::singleShot(1000, this, &Modbus::readAllInputs);
    } else if (state == QModbusDevice::UnconnectedState) {
        Logger::info("Modbus client disconnected. Will attempt reconnection.");
        GlobalErrors::setError(GlobalErrors::ModbusError);
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
