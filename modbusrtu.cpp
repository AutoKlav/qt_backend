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

    configureConnectionParameters();  // Set up connection parameters
    
    // Set up periodic reading
    readTimer.setInterval(READ_INTERVAL_MS);  // Read every 1 second
    connect(&readTimer, &QTimer::timeout, this, [this]() {
        readHoldingRegisters(CONSTANTS::TEMP, 1, 1);
        readHoldingRegisters(CONSTANTS::TEMP_K, 1, 1);
    });

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
        QVariant(QStringLiteral("COM5"))
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
        qDebug() << "RTU device not connected";
        return;
    }

    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, startAddr, count);

    if (auto *reply = modbusDevice->sendReadRequest(readUnit, slaveAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, &ModbusRTU::onReadReady);
        } else {
            delete reply;
        }
    } else {
        qDebug() << "Read request error:" << modbusDevice->errorString();
    }
}


void ModbusRTU::attemptReconnect()
{
    if (modbusDevice && modbusDevice->state() == QModbusDevice::ConnectedState)
        return;

    Logger::info("Reinitializing Modbus RTU client...");

    // 1. Clean up the old client
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        modbusDevice->deleteLater();
    }

    // 2. Re-create and configure
    modbusDevice = new QModbusRtuSerialClient(this);
    configureConnectionParameters();  
    // (move your setConnectionParameter calls into this helper)

    connect(modbusDevice, &QModbusClient::errorOccurred, this, &ModbusRTU::onErrorOccurred);
    connect(modbusDevice, &QModbusClient::stateChanged, this, &ModbusRTU::onStateChanged);

    // 3. Attempt to connect anew
    if (!modbusDevice->connectDevice()) {
        Logger::crit(QString("Reconnect failed: %1").arg(modbusDevice->errorString()));
        retryTimer.start();
    }
}


void ModbusRTU::onReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply) return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        const quint8 slaveAddress = reply->serverAddress();
        const quint16 registerAddress = unit.startAddress();

        if (unit.valueCount() >= 1) {
            quint16 rawValue = unit.value(0);
            uint scaledValue = static_cast<uint>(rawValue);

            // Use register address as the array index 'i'
            if (Sensor::mapInputPin.contains(registerAddress)) {
                Logger::info(QString("Read slave:%1 reg:%2 value:%3")
                                 .arg(slaveAddress)
                                 .arg(registerAddress)
                                 .arg(scaledValue));

                Sensor::mapInputPin[slaveAddress]->setValue(scaledValue);                
            } else {
                Logger::crit(QString("Sensor not found for slave:%1 reg:%2")
                                 .arg(slaveAddress)
                                 .arg(registerAddress));
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
        Logger::info("Modbus RTU disconnected - attempting reconnection");
        GlobalErrors::setError(GlobalErrors::ModbusError);
        readTimer.stop();  // Stop trying to read while disconnected
        retryTimer.start();
    }
}

void ModbusRTU::connectToDevice()
{
    attemptReconnect();
}


