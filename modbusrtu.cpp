#include "modbusrtu.h"
#include <QSerialPort>
#include <QVariant>
#include "constants.h"
#include "sensor.h"
#include "logger.h"
#include "globalerrors.h"

qint64 ModbusRTU::lastDataTime = 0;

ModbusRTU::ModbusRTU(QObject *parent)
    : QObject{parent},
    modbusDevice(new QModbusRtuSerialClient(this)),
    cleanupTimer(new QTimer(this))
{
    connect(modbusDevice, &QModbusClient::errorOccurred, this, &ModbusRTU::onErrorOccurred);
    connect(modbusDevice, &QModbusClient::stateChanged, this, &ModbusRTU::onStateChanged);

    configureConnectionParameters();

    // Set up periodic reading with sequential requests
    readTimer.setInterval(READ_INTERVAL_MS);
    connect(&readTimer, &QTimer::timeout, this, &ModbusRTU::processSequentialReads);

    retryTimer.setInterval(WAIT_TIME_MS);
    connect(&retryTimer, &QTimer::timeout, this, &ModbusRTU::attemptReconnect);

    // Set up a timer to periodically clean up stale replies
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
        QVariant(portName)
        );

    modbusDevice->setConnectionParameter(
        QModbusDevice::SerialBaudRateParameter,
        QVariant(baudRate)
        );

    modbusDevice->setConnectionParameter(
        QModbusDevice::SerialParityParameter,
        QVariant(parity)
        );

    modbusDevice->setConnectionParameter(
        QModbusDevice::SerialDataBitsParameter,
        QVariant(dataBits)
        );

    modbusDevice->setConnectionParameter(
        QModbusDevice::SerialStopBitsParameter,
        QVariant(stopBits)
        );
}

ModbusRTU &ModbusRTU::instance()
{
    static ModbusRTU _instance;
    return _instance;
}

void ModbusRTU::processSequentialReads()
{
    if (!modbusDevice || modbusDevice->state() != QModbusDevice::ConnectedState) {
        return;
    }

    // Clean up any stale pending replies first
    cleanupStaleReplies();

    // Send read requests with staggered delays to avoid collisions
    int delay = 0;

    // Temperature sensors
    QTimer::singleShot(delay, this, [this]() {
        readHoldingRegisters(CONSTANTS::TEMP, 1, 1);
    });
    delay += REQUEST_DELAY_MS;

    QTimer::singleShot(delay, this, [this]() {
        readHoldingRegisters(CONSTANTS::TEMP_K, 1, 1);
    });
    delay += REQUEST_DELAY_MS;

    QTimer::singleShot(delay, this, [this]() {
        readHoldingRegisters(CONSTANTS::EXPANSION_TEMP, 1, 1);
    });
    delay += REQUEST_DELAY_MS;

    // Level and pressure sensors
    QTimer::singleShot(delay, this, [this]() {
        readHoldingRegisters(CONSTANTS::TANK_WATER_LEVEL, 1, 1);
    });
    delay += REQUEST_DELAY_MS;

    QTimer::singleShot(delay, this, [this]() {
        readHoldingRegisters(CONSTANTS::PRESSURE, 1, 1);
    });

    // Digital inputs (commented out but structured)
    /*
    delay += REQUEST_DELAY_MS;
    QTimer::singleShot(delay, this, [this]() {
        readDiscreteRegisters(CONSTANTS::CWT_SLAVE_ID, CONSTANTS::DOOR_CLOSED, 1);
    });
    */
}

void ModbusRTU::readHoldingRegisters(quint8 slaveAddress, quint16 startAddr, quint16 count)
{
    sendSingleReadRequest(slaveAddress, startAddr, count, "Holding");
}

void ModbusRTU::readDiscreteRegisters(quint8 slaveAddress, quint16 startAddr, quint16 count)
{
    sendSingleReadRequest(slaveAddress, startAddr, count, "Discrete");
}

void ModbusRTU::sendSingleReadRequest(quint8 slaveAddress, quint16 startAddr, quint16 count, const QString &type)
{
    if (!modbusDevice || modbusDevice->state() != QModbusDevice::ConnectedState) {
        Logger::debug(QString("RTU device not connected - Slave: %1, Address: %2")
                          .arg(slaveAddress)
                          .arg(startAddr));
        return;
    }

    // Check if we have too many pending requests
    QMutexLocker locker(&pendingRepliesMutex);
    if (pendingReplies.count() >= MAX_PENDING_REQUESTS) {
        Logger::warn(QString("Too many pending requests (%1), skipping read - Slave: %2, Address: %3")
                         .arg(pendingReplies.count())
                         .arg(slaveAddress)
                         .arg(startAddr));
        return;
    }
    locker.unlock();

    QModbusDataUnit readUnit;
    if (type == "Holding") {
        readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, startAddr, count);
    } else if (type == "Discrete") {
        readUnit = QModbusDataUnit(QModbusDataUnit::DiscreteInputs, startAddr, count);
    } else {
        Logger::crit(QString("Unknown read type: %1").arg(type));
        return;
    }

    Logger::debug(QString("Sending %1 read - Slave: %2, Address: %3, Count: %4")
                      .arg(type)
                      .arg(slaveAddress)
                      .arg(startAddr)
                      .arg(count));

    if (auto *reply = modbusDevice->sendReadRequest(readUnit, slaveAddress)) {
        if (!reply->isFinished()) {
            setupReplyConnections(reply, type, slaveAddress, startAddr);
        } else {
            Logger::debug("Reply finished immediately, deleting");
            delete reply;
        }
    } else {
        Logger::crit(QString("Read request failed - Slave: %1, Address: %2 - Error: %3")
                         .arg(slaveAddress)
                         .arg(startAddr)
                         .arg(modbusDevice->errorString()));
    }
}

void ModbusRTU::setupReplyConnections(QModbusReply *reply, const QString &type, quint8 slaveAddress, quint16 startAddr)
{
    QMutexLocker locker(&pendingRepliesMutex);
    pendingReplies.insert(reply);
    locker.unlock();

    // Store request info for better logging
    reply->setProperty("slaveAddress", slaveAddress);
    reply->setProperty("startAddress", startAddr);
    reply->setProperty("requestType", type);

    if (type == "Holding") {
        connect(reply, &QModbusReply::finished, this, &ModbusRTU::onReadReady);
    } else if (type == "Discrete") {
        connect(reply, &QModbusReply::finished, this, &ModbusRTU::onDigitalInputReady);
    }

    // Set individual timeout for this request
    QTimer::singleShot(REQUEST_TIMEOUT_MS, this, [this, reply]() {
        QMutexLocker locker(&pendingRepliesMutex);
        if (pendingReplies.contains(reply)) {
            if (!reply->isFinished()) {
                quint8 slaveAddr = reply->property("slaveAddress").toUInt();
                quint16 startAddr = reply->property("startAddress").toUInt();
                QString reqType = reply->property("requestType").toString();

                Logger::warn(QString("Request timeout - Type: %1, Slave: %2, Address: %3")
                                 .arg(reqType)
                                 .arg(slaveAddr)
                                 .arg(startAddr));

                reply->deleteLater(); // Properly abort the request
            }
            pendingReplies.remove(reply);
            reply->deleteLater();
        }
    });
}

void ModbusRTU::onReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply) return;

    // Remove from pending set with mutex protection
    QMutexLocker locker(&pendingRepliesMutex);
    if (!pendingReplies.contains(reply)) {
        // Reply was already cleaned up by timeout
        reply->deleteLater();
        return;
    }
    pendingReplies.remove(reply);
    locker.unlock();

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        const quint8 slaveAddress = reply->serverAddress();
        const quint16 startAddress = unit.startAddress();

        if (unit.valueCount() >= 1) {
            quint16 rawValue = unit.value(0);
            uint scaledValue = static_cast<uint>(rawValue);

            if (Sensor::mapInputPin.contains(slaveAddress)) {
                Logger::info(QString("Read holding register - Slave: %1, Address: %2, Value: %3")
                                 .arg(slaveAddress)
                                 .arg(startAddress)
                                 .arg(scaledValue));

                Sensor::mapInputPin[slaveAddress]->setValue(scaledValue);
            } else {
                Logger::warn(QString("Sensor not found for slave: %1, address: %2")
                                 .arg(slaveAddress)
                                 .arg(startAddress));
                GlobalErrors::setError(GlobalErrors::DbError);
            }

            lastDataTime = QDateTime::currentMSecsSinceEpoch();
        }
    } else {
        // Only log non-timeout errors (timeouts are handled in timeout handler)
        if (reply->error() != QModbusDevice::TimeoutError) {
            quint8 slaveAddr = reply->property("slaveAddress").toUInt();
            quint16 startAddr = reply->property("startAddress").toUInt();

            Logger::warn(QString("Read error - Slave: %1, Address: %2 - Error: %3")
                             .arg(slaveAddr)
                             .arg(startAddr)
                             .arg(reply->errorString()));
        }

        // If we're getting consistent errors, consider reconnecting
        static int errorCount = 0;
        errorCount++;
        if (errorCount > 5) {
            Logger::warn("Multiple consecutive errors, attempting reconnect");
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

    // Remove from pending set with mutex protection
    QMutexLocker locker(&pendingRepliesMutex);
    if (!pendingReplies.contains(reply)) {
        reply->deleteLater();
        return;
    }
    pendingReplies.remove(reply);
    locker.unlock();

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        const quint8 slaveAddress = reply->serverAddress();
        const quint16 startAddress = unit.startAddress();

        if (unit.valueCount() >= 1) {
            quint16 rawValue = unit.value(0);
            uint scaledValue = static_cast<uint>(rawValue);

            const auto shiftedAddress = startAddress + CONSTANTS::DIGITAL_INPUT_SHIFT;

            if (Sensor::mapInputPin.contains(shiftedAddress)) {
                Logger::info(QString("Read discrete input - Slave: %1, Address: %2, Value: %3")
                                 .arg(slaveAddress)
                                 .arg(startAddress)
                                 .arg(scaledValue));

                Sensor::mapInputPin[shiftedAddress]->setValue(scaledValue);
            } else {
                Logger::warn(QString("Sensor not found for shifted address: %1 (original: %2)")
                                 .arg(shiftedAddress)
                                 .arg(startAddress));
                GlobalErrors::setError(GlobalErrors::DbError);
            }

            lastDataTime = QDateTime::currentMSecsSinceEpoch();
        }
    } else {
        // Only log non-timeout errors
        if (reply->error() != QModbusDevice::TimeoutError) {
            quint8 slaveAddr = reply->property("slaveAddress").toUInt();
            quint16 startAddr = reply->property("startAddress").toUInt();

            Logger::warn(QString("Discrete read error - Slave: %1, Address: %2 - Error: %3")
                             .arg(slaveAddr)
                             .arg(startAddr)
                             .arg(reply->errorString()));
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

    // Log the write attempt with details
    Logger::info(QString("Attempting to write coil - Slave: %1, Address: %2, Value: %3")
                     .arg(slaveAddress)
                     .arg(coilAddress)
                     .arg(value ? "ON" : "OFF"));

    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, slaveAddress)) {
        QMutexLocker locker(&pendingRepliesMutex);
        pendingReplies.insert(reply);
        locker.unlock();

        // Store request info
        reply->setProperty("slaveAddress", slaveAddress);
        reply->setProperty("coilAddress", coilAddress);
        reply->setProperty("value", value);

        connect(reply, &QModbusReply::finished, this, [this, reply]() {
            QMutexLocker locker(&pendingRepliesMutex);
            if (!pendingReplies.contains(reply)) {
                reply->deleteLater();
                return;
            }
            pendingReplies.remove(reply);
            locker.unlock();

            quint8 slaveAddr = reply->property("slaveAddress").toUInt();
            quint16 coilAddr = reply->property("coilAddress").toUInt();
            bool val = reply->property("value").toBool();

            if (reply->error() != QModbusDevice::NoError) {
                Logger::crit(QString("Write coil error - Slave: %1, Address: %2, Value: %3 - Error: %4")
                                 .arg(slaveAddr)
                                 .arg(coilAddr)
                                 .arg(val ? "ON" : "OFF")
                                 .arg(reply->errorString()));
                GlobalErrors::setError(GlobalErrors::ModbusWriteCoilError);
            } else {
                Logger::info(QString("Successfully wrote coil - Slave: %1, Address: %2, Value: %3")
                                 .arg(slaveAddr)
                                 .arg(coilAddr)
                                 .arg(val ? "ON" : "OFF"));
            }
            reply->deleteLater();
        });

        // Set timeout for write request
        QTimer::singleShot(REQUEST_TIMEOUT_MS, this, [this, reply]() {
            QMutexLocker locker(&pendingRepliesMutex);
            if (pendingReplies.contains(reply)) {
                quint8 slaveAddr = reply->property("slaveAddress").toUInt();
                quint16 coilAddr = reply->property("coilAddress").toUInt();
                bool val = reply->property("value").toBool();

                if (!reply->isFinished()) {
                    Logger::warn(QString("Write coil timeout - Slave: %1, Address: %2, Value: %3")
                                     .arg(slaveAddr)
                                     .arg(coilAddr)
                                     .arg(val ? "ON" : "OFF"));
                    reply->deleteLater();
                }
                pendingReplies.remove(reply);
                reply->deleteLater();
            }
        });
    } else {
        Logger::crit(QString("Write coil request failed - Slave: %1, Address: %2, Value: %3 - Error: %4")
                         .arg(slaveAddress)
                         .arg(coilAddress)
                         .arg(value ? "ON" : "OFF")
                         .arg(modbusDevice->errorString()));
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

    // Clean up the old client with proper disconnection
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        // Small delay to ensure clean disconnect
        QTimer::singleShot(100, this, [this]() {
            modbusDevice->deleteLater();
            modbusDevice = nullptr;

            // Re-create and configure
            modbusDevice = new QModbusRtuSerialClient(this);
            configureConnectionParameters();
            connect(modbusDevice, &QModbusClient::errorOccurred, this, &ModbusRTU::onErrorOccurred);
            connect(modbusDevice, &QModbusClient::stateChanged, this, &ModbusRTU::onStateChanged);

            if (!modbusDevice->connectDevice()) {
                Logger::crit(QString("Reconnect failed: %1").arg(modbusDevice->errorString()));
                retryTimer.start();
            }
        });
    } else {
        // First time initialization
        modbusDevice = new QModbusRtuSerialClient(this);
        configureConnectionParameters();
        connect(modbusDevice, &QModbusClient::errorOccurred, this, &ModbusRTU::onErrorOccurred);
        connect(modbusDevice, &QModbusClient::stateChanged, this, &ModbusRTU::onStateChanged);

        if (!modbusDevice->connectDevice()) {
            Logger::crit(QString("Initial connection failed: %1").arg(modbusDevice->errorString()));
            retryTimer.start();
        }
    }
}

void ModbusRTU::cleanupPendingReplies()
{
    QMutexLocker locker(&pendingRepliesMutex);
    for (QModbusReply *reply : pendingReplies) {
        if (reply) {
            if (!reply->isFinished()) {
                reply->deleteLater();
            }
            reply->deleteLater();
        }
    }
    pendingReplies.clear();
}

void ModbusRTU::cleanupStaleReplies()
{
    QMutexLocker locker(&pendingRepliesMutex);
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

    QMutexLocker locker(&pendingRepliesMutex);
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
