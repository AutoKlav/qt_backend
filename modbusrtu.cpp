#include "modbusrtu.h"
#include <QSerialPort>
#include <QVariant>
#include "constants.h"
#include "sensor.h"
#include "logger.h"
#include "globalerrors.h"

qint64 ModbusRTU::lastDataTime = 0;

ModbusRTU::ModbusRTU(QObject *parent)
    : QObject{parent}, modbusDevice(new QModbusRtuSerialClient(this))
{
    connect(modbusDevice, &QModbusClient::errorOccurred, this, &ModbusRTU::onErrorOccurred);
    connect(modbusDevice, &QModbusClient::stateChanged, this, &ModbusRTU::onStateChanged);

    configureConnectionParameters();

    // Set up periodic reading
    readTimer.setInterval(READ_INTERVAL_MS);
    connect(&readTimer, &QTimer::timeout, this, [this]() {
        // Only send new requests if we're connected
        if (!modbusDevice || modbusDevice->state() != QModbusDevice::ConnectedState) {
            return;
        }

        // Clean up any stale pending replies first
        cleanupStaleReplies();

        // Read all temperature values
        readHoldingRegisters(CONSTANTS::TEMP, 1, 1);
        readHoldingRegisters(CONSTANTS::TEMP_K, 1, 1);
        readHoldingRegisters(CONSTANTS::EXPANSION_TEMP, 1, 1);
        // readHoldingRegisters(CONSTANTS::HEATER_TEMP, 1, 1);
        // readHoldingRegisters(CONSTANTS::TANK_TEMP, 1, 1);

        // Read level and pressure values
        readHoldingRegisters(CONSTANTS::TANK_WATER_LEVEL, 1, 1);
        // readHoldingRegisters(CONSTANTS::STEAM_PRESSURE, 1, 1);
        readHoldingRegisters(CONSTANTS::PRESSURE, 1, 1);

        // Read digital input directly from cwt digital input pins
        // readDiscreteRegisters(CONSTANTS::CWT_SLAVE_ID, CONSTANTS::DOOR_CLOSED, 1);
        // readDiscreteRegisters(CONSTANTS::CWT_SLAVE_ID, CONSTANTS::BURNER_FAULT, 1);
        // readDiscreteRegisters(CONSTANTS::CWT_SLAVE_ID, CONSTANTS::WATER_SHORTAGE, 1);
    });

    retryTimer.setInterval(WAIT_TIME_MS);
    connect(&retryTimer, &QTimer::timeout, this, &ModbusRTU::attemptReconnect);

    // Set up a timer to periodically clean up stale replies
    QTimer *cleanupTimer = new QTimer(this);
    cleanupTimer->setInterval(5000); // Clean up every 5 seconds
    connect(cleanupTimer, &QTimer::timeout, this, &ModbusRTU::cleanupStaleReplies);
    cleanupTimer->start();

    attemptReconnect();
}

void ModbusRTU::configureConnectionParameters()
{
    if (!modbusDevice) {
        return;
    }

    modbusDevice->setConnectionParameter(
        QModbusDevice::SerialPortNameParameter,
        QVariant(QStringLiteral("COM7"))
        );

    modbusDevice->setConnectionParameter(
        QModbusDevice::SerialBaudRateParameter,
        QVariant(static_cast<int>(QSerialPort::Baud9600))
        );

    modbusDevice->setConnectionParameter(
        QModbusDevice::SerialParityParameter,
        QVariant(static_cast<int>(QSerialPort::OddParity))
        );

    modbusDevice->setConnectionParameter(
        QModbusDevice::SerialDataBitsParameter,
        QVariant(static_cast<int>(QSerialPort::Data8))
        );

    modbusDevice->setConnectionParameter(
        QModbusDevice::SerialStopBitsParameter,
        QVariant(static_cast<int>(QSerialPort::OneStop))
        );
}

ModbusRTU &ModbusRTU::instance()
{
    static ModbusRTU _instance;
    return _instance;
}

void ModbusRTU::readHoldingRegisters(quint8 slaveAddress, quint16 startAddr, quint16 count)
{
    if (!modbusDevice || modbusDevice->state() != QModbusDevice::ConnectedState) {
        qDebug() << "RTU device not connected - Slave:" << slaveAddress;
        return;
    }

    // Check if we have too many pending requests
    if (pendingReplies.count() >= MAX_PENDING_REQUESTS) {
        qDebug() << "Too many pending requests, skipping read for slave:" << slaveAddress;
        return;
    }

    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, startAddr, count);

    if (auto *reply = modbusDevice->sendReadRequest(readUnit, slaveAddress)) {
        if (!reply->isFinished()) {
            setupReplyConnections(reply, "Holding");
        } else {
            delete reply;
        }
    } else {
        qDebug() << "Read request error - Slave" << slaveAddress << ":" << modbusDevice->errorString();
    }
}

void ModbusRTU::readDiscreteRegisters(quint8 slaveAddress, quint16 startAddr, quint16 count)
{
    if (!modbusDevice || modbusDevice->state() != QModbusDevice::ConnectedState) {
        qDebug() << "RTU device not connected - Slave:" << slaveAddress;
        return;
    }

    // Check if we have too many pending requests
    if (pendingReplies.count() >= MAX_PENDING_REQUESTS) {
        qDebug() << "Too many pending requests, skipping read for slave:" << slaveAddress;
        return;
    }

    QModbusDataUnit readUnit(QModbusDataUnit::DiscreteInputs, startAddr, count);

    if (auto *reply = modbusDevice->sendReadRequest(readUnit, slaveAddress)) {
        if (!reply->isFinished()) {
            setupReplyConnections(reply, "Discrete");
        } else {
            delete reply;
        }
    } else {
        qDebug() << "Read request error - Slave" << slaveAddress << ":" << modbusDevice->errorString();
    }
}

void ModbusRTU::setupReplyConnections(QModbusReply *reply, const QString &type)
{
    pendingReplies.insert(reply);

    if (type == "Holding") {
        connect(reply, &QModbusReply::finished, this, &ModbusRTU::onReadReady);
    } else if (type == "Discrete") {
        connect(reply, &QModbusReply::finished, this, &ModbusRTU::onDigitalInputReady);
    }

    // Set individual timeout for this request
    QTimer::singleShot(REQUEST_TIMEOUT_MS, this, [this, reply]() {
        if (pendingReplies.contains(reply) && !reply->isFinished()) {
            qDebug() << "Request timeout, cleaning up reply";
            pendingReplies.remove(reply);
            reply->deleteLater();
        }
    });
}

void ModbusRTU::onReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply) return;

    // Remove from pending set
    pendingReplies.remove(reply);

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        const quint8 slaveAddress = reply->serverAddress();
        const quint16 startAddress = unit.startAddress();

        if (unit.valueCount() >= 1) {
            quint16 rawValue = unit.value(0);
            uint scaledValue = static_cast<uint>(rawValue);

            if (Sensor::mapInputPin.contains(slaveAddress)) {
                Logger::info(QString("Read slave:%1 address:%2 value:%3")
                                 .arg(slaveAddress)
                                 .arg(startAddress)
                                 .arg(scaledValue));

                Sensor::mapInputPin[slaveAddress]->setValue(scaledValue);
            } else {
                Logger::crit(QString("Sensor not found for slave:%1 address:%2")
                                 .arg(slaveAddress)
                                 .arg(startAddress));
                GlobalErrors::setError(GlobalErrors::DbError);
            }

            lastDataTime = QDateTime::currentMSecsSinceEpoch();
        }
    } else {
        // Only log non-timeout errors
        if (reply->error() != QModbusDevice::TimeoutError) {
            qDebug() << "Read error:" << reply->errorString();
        }

        // If we're getting consistent errors, consider reconnecting
        static int errorCount = 0;
        errorCount++;
        if (errorCount > 5) {
            qDebug() << "Multiple consecutive errors, attempting reconnect";
            errorCount = 0;
            QTimer::singleShot(0, this, &ModbusRTU::attemptReconnect);
        }
    }
    reply->deleteLater();
}

void ModbusRTU::onDigitalInputReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply) return;

    // Remove from pending set
    pendingReplies.remove(reply);

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        const quint8 slaveAddress = reply->serverAddress();
        const quint16 startAddress = unit.startAddress();

        if (unit.valueCount() >= 1) {
            quint16 rawValue = unit.value(0);
            uint scaledValue = static_cast<uint>(rawValue);

            const auto shiftedAddress = startAddress + CONSTANTS::DIGITAL_INPUT_SHIFT;

            if (Sensor::mapInputPin.contains(shiftedAddress)) {
                Logger::info(QString("Read slave:%1 address:%2 value:%3")
                                 .arg(slaveAddress)
                                 .arg(startAddress)
                                 .arg(scaledValue));

                Sensor::mapInputPin[shiftedAddress]->setValue(scaledValue);
            } else {
                Logger::crit(QString("Sensor not found for slave:%1 address:%2")
                                 .arg(slaveAddress)
                                 .arg(startAddress));
                GlobalErrors::setError(GlobalErrors::DbError);
            }

            lastDataTime = QDateTime::currentMSecsSinceEpoch();
        }
    } else {
        // Only log non-timeout errors
        if (reply->error() != QModbusDevice::TimeoutError) {
            qDebug() << "Read error:" << reply->errorString();
        }
    }
    reply->deleteLater();
}

void ModbusRTU::writeSingleCoil(quint8 slaveAddress, quint16 coilAddress, bool value)
{
    if (!modbusDevice || modbusDevice->state() != QModbusDevice::ConnectedState) {
        Logger::crit("Modbus RTU device not connected. Cannot write to coil.");
        return;
    }

    QModbusDataUnit writeUnit(QModbusDataUnit::Coils, coilAddress, 1);
    writeUnit.setValue(0, value ? static_cast<quint16>(CONSTANTS::MODBUS_COIL_ON) : static_cast<quint16>(CONSTANTS::MODBUS_COIL_OFF));

    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, slaveAddress)) {
        pendingReplies.insert(reply);
        connect(reply, &QModbusReply::finished, this, [this, reply]() {
            pendingReplies.remove(reply);

            if (reply->error() != QModbusDevice::NoError) {
                Logger::crit(QString("Write coil error: %1").arg(reply->errorString()));
                GlobalErrors::setError(GlobalErrors::ModbusWriteCoilError);
            } else {
                Logger::info("Successfully wrote coil value");
            }
            reply->deleteLater();
        });

        // Set timeout for write request
        QTimer::singleShot(REQUEST_TIMEOUT_MS, this, [this, reply]() {
            if (pendingReplies.contains(reply) && !reply->isFinished()) {
                pendingReplies.remove(reply);
                reply->deleteLater();
            }
        });
    } else {
        Logger::crit(QString("Write coil request failed: %1").arg(modbusDevice->errorString()));
    }
}

void ModbusRTU::attemptReconnect()
{
    if (modbusDevice && modbusDevice->state() == QModbusDevice::ConnectedState) {
        return;
    }

    Logger::info("Reinitializing Modbus RTU client...");

    // Clean up all pending replies first
    cleanupPendingReplies();

    // Stop timers during reconnection
    readTimer.stop();
    retryTimer.stop();

    // Clean up the old client
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        modbusDevice->deleteLater();
        modbusDevice = nullptr;
    }

    // Re-create and configure
    modbusDevice = new QModbusRtuSerialClient(this);
    configureConnectionParameters();
    connect(modbusDevice, &QModbusClient::errorOccurred, this, &ModbusRTU::onErrorOccurred);
    connect(modbusDevice, &QModbusClient::stateChanged, this, &ModbusRTU::onStateChanged);

    if (!modbusDevice->connectDevice()) {
        Logger::crit(QString("Reconnect failed: %1").arg(modbusDevice->errorString()));
        retryTimer.start();
    }
}

void ModbusRTU::cleanupPendingReplies()
{
    for (QModbusReply *reply : pendingReplies) {
        if (reply) {
            pendingReplies.remove(reply);
            reply->deleteLater();
        }
    }
    pendingReplies.clear();
}

void ModbusRTU::cleanupStaleReplies()
{
    // Remove any replies that are finished but still in the set (shouldn't happen, but just in case)
    QSet<QModbusReply*> toRemove;
    for (QModbusReply *reply : pendingReplies) {
        if (reply && reply->isFinished()) {
            toRemove.insert(reply);
            reply->deleteLater();
        }
    }
    for (QModbusReply *reply : toRemove) {
        pendingReplies.remove(reply);
    }
}

bool ModbusRTU::isRequestPending(quint8 slaveAddress, quint16 startAddr)
{
    Q_UNUSED(slaveAddress)
    Q_UNUSED(startAddr)

    // Simple throttle - if we have too many pending requests, don't add more
    return pendingReplies.count() >= MAX_PENDING_REQUESTS;
}

void ModbusRTU::onErrorOccurred(QModbusDevice::Error error)
{
    if (error != QModbusDevice::NoError) {
        Logger::crit(QString("Modbus RTU error: %1").arg(modbusDevice->errorString()));
        if (modbusDevice->state() != QModbusDevice::ConnectedState) {
            readTimer.stop();
            retryTimer.start();
        }
    }
}

void ModbusRTU::onStateChanged(QModbusDevice::State state)
{
    Logger::info(QString("Modbus RTU state changed to: %1").arg(state));

    if (state == QModbusDevice::ConnectedState) {
        Logger::info("Modbus RTU successfully connected");
        GlobalErrors::removeError(GlobalErrors::ModbusError);
        retryTimer.stop();
        readTimer.start();  // Restart reading
        // Clean up any stale replies when we reconnect
        cleanupStaleReplies();
    }
    else if (state == QModbusDevice::UnconnectedState) {
        Logger::info("Modbus RTU disconnected - attempting reconnection");
        GlobalErrors::setError(GlobalErrors::ModbusError);
        readTimer.stop();  // Stop trying to read while disconnected
        cleanupPendingReplies(); // Clean up any pending requests
        retryTimer.start();
    }
    else if (state == QModbusDevice::ConnectingState) {
        Logger::info("Modbus RTU connecting...");
        readTimer.stop(); // Stop reading while connecting
    }
}

void ModbusRTU::connectToDevice()
{
    attemptReconnect();
}
