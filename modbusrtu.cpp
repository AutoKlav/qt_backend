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

    // Set up periodic reading with sequential processing
    readTimer.setInterval(READ_INTERVAL_MS);
    connect(&readTimer, &QTimer::timeout, this, &ModbusRTU::startSequentialReading);

    // Request processing timer
    requestTimer.setInterval(REQUEST_DELAY_MS);
    connect(&requestTimer, &QTimer::timeout, this, &ModbusRTU::processNextRequest);

    // Initialize retry timer
    retryTimer.setInterval(WAIT_TIME_MS);
    connect(&retryTimer, &QTimer::timeout, this, &ModbusRTU::attemptReconnect);

    // Start initial connection
    attemptReconnect();
}

void ModbusRTU::configureConnectionParameters()
{
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

    // Set timeout and number of retries
    modbusDevice->setTimeout(1000);
    modbusDevice->setNumberOfRetries(3);
}

ModbusRTU &ModbusRTU::instance()
{
    static ModbusRTU _instance;
    return _instance;
}

void ModbusRTU::queueRequest(const ModbusRequest &request)
{
     // Safety check - prevent unlimited queue growth
    if (requestQueue.size() >= MAX_QUEUE_SIZE) {
        Logger::info("Request queue full - discarding request");
        return;
    }
    
    requestQueue.enqueue(request);
    if (!requestTimer.isActive() && !isProcessingRequest) {
        requestTimer.start();
    }
}

void ModbusRTU::processNextRequest()
{
    if (isProcessingRequest || requestQueue.isEmpty() || !isConnected()) {
        if (requestQueue.isEmpty()) {
            requestTimer.stop();
        }
        return;
    }

    isProcessingRequest = true;
    ModbusRequest request = requestQueue.dequeue();

    QModbusDataUnit unit(request.registerType, request.startAddress, request.count);

    if (auto *reply = modbusDevice->sendReadRequest(unit, request.slaveAddress)) {
        Logger::debug(QString("Sent request: %1 - Slave: %2, Addr: %3")
                          .arg(request.description)
                          .arg(request.slaveAddress)
                          .arg(request.startAddress));

        connect(reply, &QModbusReply::finished, this, [this, request, reply]() {
            if (reply->error() == QModbusDevice::NoError) {
                request.callback(reply->result(), request.slaveAddress);
            } else {
                Logger::info(QString("Request failed: %1 - Error: %2")
                                    .arg(request.description)
                                    .arg(reply->errorString()));

                // Optional: Re-queue failed request (be careful with this)
                // if (retryCount < MAX_RETRIES) {
                //     queueRequest(request);
                // }
            }
            reply->deleteLater();
            isProcessingRequest = false;

            // Process next request after a short delay
            QTimer::singleShot(REQUEST_DELAY_MS, this, &ModbusRTU::processNextRequest);
        });
    } else {
        Logger::info(QString("Failed to send request: %1").arg(request.description));
        isProcessingRequest = false;
        QTimer::singleShot(REQUEST_DELAY_MS, this, &ModbusRTU::processNextRequest);
    }
}

void ModbusRTU::startSequentialReading()
{
    // Don't queue new requests if we're not connected
    if (!isConnected()) {
        return;
    }
    
    // Also check if queue is already too large (safety measure)
    if (requestQueue.size() > MAX_QUEUE_SIZE) {
        Logger::info("Request queue too large - clearing stale requests");
        requestQueue.clear();
    }
    
    if (!requestQueue.isEmpty()) {
        return; // Skip if still processing previous requests
    }

    // Queue all sensor reads sequentially
    queueRequest({CONSTANTS::TEMP, QModbusDataUnit::HoldingRegisters, 1, 1,
                  [this](const QModbusDataUnit& unit, quint8 slaveAddr) {
                      handleTemperatureReading(unit, slaveAddr);
                  }, "Temperature"});

    queueRequest({CONSTANTS::TEMP_K, QModbusDataUnit::HoldingRegisters, 1, 1,
                  [this](const QModbusDataUnit& unit, quint8 slaveAddr) {
                      handleTemperatureReading(unit, slaveAddr);
                  }, "Temperature Kelvin"});

    queueRequest({CONSTANTS::EXPANSION_TEMP, QModbusDataUnit::HoldingRegisters, 1, 1,
                  [this](const QModbusDataUnit& unit, quint8 slaveAddr) {
                      handleTemperatureReading(unit, slaveAddr);
                  }, "Expansion Temperature"});

    queueRequest({CONSTANTS::TANK_WATER_LEVEL, QModbusDataUnit::HoldingRegisters, 1, 1,
                  [this](const QModbusDataUnit& unit, quint8 slaveAddr) {
                      handleLevelReading(unit, slaveAddr);
                  }, "Tank Water Level"});

    queueRequest({CONSTANTS::PRESSURE, QModbusDataUnit::HoldingRegisters, 1, 1,
                  [this](const QModbusDataUnit& unit, quint8 slaveAddr) {
                      handlePressureReading(unit, slaveAddr);
                  }, "Pressure"});

    // Digital inputs (uncomment when needed)
    /*
    queueRequest({CONSTANTS::CWT_SLAVE_ID, QModbusDataUnit::DiscreteInputs, CONSTANTS::DOOR_CLOSED, 1,
        [this](const QModbusDataUnit& unit, quint8 slaveAddr) {
            handleDigitalInputReading(unit, slaveAddr, CONSTANTS::DOOR_CLOSED);
        }, "Door Status"});
    */
}

void ModbusRTU::readHoldingRegisters(quint8 slaveAddress, quint16 startAddr, quint16 count, const QString &description)
{
    queueRequest({slaveAddress, QModbusDataUnit::HoldingRegisters, startAddr, count,
                  [this](const QModbusDataUnit& unit, quint8 slaveAddr) {
                      // Use the new handler system instead of onReadReady
                      // For holding registers, we can use the start address from the unit
                      handleTemperatureReading(unit, slaveAddr);
                  }, description.isEmpty() ? "Holding Register Read" : description});
}

void ModbusRTU::readDiscreteRegisters(quint8 slaveAddress, quint16 startAddr, quint16 count, const QString &description)
{
    // Capture startAddr by value in the lambda
    queueRequest({slaveAddress, QModbusDataUnit::DiscreteInputs, startAddr, count,
                  [this, startAddr](const QModbusDataUnit& unit, quint8 slaveAddr) {
                      // Use the new handler system instead of onDigitalInputReady
                      handleDigitalInputReading(unit, slaveAddr, startAddr);
                  }, description.isEmpty() ? "Discrete Input Read" : description});
}

void ModbusRTU::handleTemperatureReading(const QModbusDataUnit &unit, quint8 slaveAddress)
{
    if (unit.valueCount() >= 1) {
        quint16 rawValue = unit.value(0);
        uint scaledValue = static_cast<uint>(rawValue);

        if (Sensor::mapInputPin.contains(slaveAddress)) {
            Logger::info(QString("Temperature - Slave:%1 Address:%2 Value:%3")
                             .arg(slaveAddress)
                             .arg(unit.startAddress())
                             .arg(scaledValue));

            Sensor::mapInputPin[slaveAddress]->setValue(scaledValue);
            lastDataTime = QDateTime::currentMSecsSinceEpoch();
        } else {
            Logger::info(QString("Sensor not found for slave:%1").arg(slaveAddress));
        }
    }
}

void ModbusRTU::handlePressureReading(const QModbusDataUnit &unit, quint8 slaveAddress)
{
    if (unit.valueCount() >= 1) {
        quint16 rawValue = unit.value(0);
        uint scaledValue = static_cast<uint>(rawValue)/10;

        if (Sensor::mapInputPin.contains(slaveAddress)) {
            Logger::info(QString("Pressure - Slave:%1 Address:%2 Value:%3")
                             .arg(slaveAddress)
                             .arg(unit.startAddress())
                             .arg(scaledValue));

            Sensor::mapInputPin[slaveAddress]->setValue(scaledValue);
            lastDataTime = QDateTime::currentMSecsSinceEpoch();
        }
    }
}

void ModbusRTU::handleLevelReading(const QModbusDataUnit &unit, quint8 slaveAddress)
{
    if (unit.valueCount() >= 1) {
        quint16 rawValue = unit.value(0);
        uint scaledValue = static_cast<uint>(rawValue);

        if (Sensor::mapInputPin.contains(slaveAddress)) {
            Logger::info(QString("Level - Slave:%1 Address:%2 Value:%3")
                             .arg(slaveAddress)
                             .arg(unit.startAddress())
                             .arg(scaledValue));

            Sensor::mapInputPin[slaveAddress]->setValue(scaledValue);
            lastDataTime = QDateTime::currentMSecsSinceEpoch();
        }
    }
}

void ModbusRTU::handleDigitalInputReading(const QModbusDataUnit &unit, quint8 slaveAddress, quint16 startAddress)
{
    if (unit.valueCount() >= 1) {
        quint16 rawValue = unit.value(0);
        uint scaledValue = static_cast<uint>(rawValue);

        const auto shiftedAddress = startAddress + CONSTANTS::DIGITAL_INPUT_SHIFT;

        if (Sensor::mapInputPin.contains(shiftedAddress)) {
            Logger::info(QString("Digital Input - Slave:%1 Address:%2 Value:%3")
                             .arg(slaveAddress)
                             .arg(startAddress)
                             .arg(scaledValue));

            Sensor::mapInputPin[shiftedAddress]->setValue(scaledValue);
            lastDataTime = QDateTime::currentMSecsSinceEpoch();
        }
    }
}

void ModbusRTU::attemptReconnect()
{
    if (isConnected()) return;

    // Exponential backoff
    int backoffDelay = qMin(WAIT_TIME_MS * (1 << qMin(retryCount, 5)), 30000);
    retryTimer.setInterval(backoffDelay);
    retryCount++;

    Logger::info(QString("Attempting Modbus RTU reconnection (attempt %1, delay %2ms)...")
                     .arg(retryCount).arg(backoffDelay));

    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        modbusDevice->deleteLater();
    }

    modbusDevice = new QModbusRtuSerialClient(this);
    configureConnectionParameters();

    connect(modbusDevice, &QModbusClient::errorOccurred, this, &ModbusRTU::onErrorOccurred);
    connect(modbusDevice, &QModbusClient::stateChanged, this, &ModbusRTU::onStateChanged);

    if (!modbusDevice->connectDevice()) {
        Logger::crit(QString("Reconnect failed: %1").arg(modbusDevice->errorString()));
        retryTimer.start();
    } else {
        retryCount = 0; // Reset on successful connection
    }
}

void ModbusRTU::disconnectDevice()
{
    readTimer.stop();
    retryTimer.stop();
    requestTimer.stop();
    requestQueue.clear();

    if (modbusDevice) {
        modbusDevice->disconnectDevice();
    }
}

// FIXED writeSingleCoil - added missing description parameter
void ModbusRTU::writeSingleCoil(quint8 slaveAddress, quint16 coilAddress, bool value, const QString &description)
{
    if (!isConnected()) {
        Logger::crit("Modbus RTU device not connected. Cannot write to coil.");
        return;
    }

    QModbusDataUnit writeUnit(QModbusDataUnit::Coils, coilAddress, 1);
    writeUnit.setValue(0, value ? static_cast<quint16>(CONSTANTS::MODBUS_COIL_ON) : static_cast<quint16>(CONSTANTS::MODBUS_COIL_OFF));
    
    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, slaveAddress)) {
        connect(reply, &QModbusReply::finished, this, [this, reply, description]() {
            if (reply->error() != QModbusDevice::NoError) {
                Logger::crit(QString("Write coil error: %1 - %2").arg(description).arg(reply->errorString()));
                GlobalErrors::setError(GlobalErrors::ModbusWriteCoilError);
            } else {
                Logger::info(QString("Successfully wrote coil: %1").arg(description));
            }
            reply->deleteLater();
        });
    } else {
        Logger::crit(QString("Write coil request failed: %1 - %2").arg(description).arg(modbusDevice->errorString()));
    }
}

void ModbusRTU::onReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply) return;

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
        qDebug() << "Read error:" << reply->errorString();
    }
    reply->deleteLater();
}

void ModbusRTU::onDigitalInputReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply) return;

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
        qDebug() << "Read error:" << reply->errorString();
    }
    reply->deleteLater();
}

void ModbusRTU::onErrorOccurred(QModbusDevice::Error error)
{
    if (error != QModbusDevice::NoError) {
        Logger::crit(QString("Modbus RTU error: %1").arg(modbusDevice->errorString()));
        if (modbusDevice->state() != QModbusDevice::ConnectedState) {
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
    }
    else if (state == QModbusDevice::UnconnectedState) {
        Logger::info("Modbus RTU disconnected - clearing request queue");
        GlobalErrors::setError(GlobalErrors::ModbusError);
        readTimer.stop();  // Stop trying to read while disconnected
        
        // CLEAR THE QUEUE when disconnected
        requestQueue.clear();
        isProcessingRequest = false;
        
        retryTimer.start();
    }
}

void ModbusRTU::connectToDevice()
{
    attemptReconnect();
}