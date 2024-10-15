#include "serial.h"

#include "sensor.h"
#include "logger.h"
#include "globalerrors.h"

#include <QTimer>

Serial::Serial(QObject *parent)
    : QObject{parent}, m_serial(new QSerialPort(this))
{
    // Initialize the retry timer with a 5-second interval
    m_retryTimer.setInterval(WAIT_TIME_MS);

    // Connect read data slot
    connect(m_serial, &QSerialPort::readyRead, this, &Serial::readData);

    // When error appears, try to reconnect
    connect(m_serial, &QSerialPort::errorOccurred, this, &Serial::tryToReconnectWhenErrorAppears);

    // If reconnect fails, try to reconnect again using timer
    connect(&m_retryTimer, &QTimer::timeout, this, [this]() {
        Logger::debug("Attempting to reconnect to the serial port...");

        if (m_serial->open(QIODevice::ReadWrite)) {
            Logger::debug("Reconnected successfully.");
            GlobalErrors::removeError(GlobalErrors::SerialError);
            m_retryTimer.stop();  // Stop retrying once successfully connected
        } else {
            Logger::crit(QString("Failed to reconnect, error: %1").arg(m_serial->errorString()));
        }
    });    
}

void Serial::readData()
{
    // Save data to buffer
    m_buffer.append(m_serial->readAll());

    // Call function to parse data
    parseData();
}


void Serial::parseData()
{
    // Find last data instance
    int endIndex = m_buffer.lastIndexOf("]");
    if (endIndex == -1)
        return;

    // Find beginning of last data instance
    int startIndex = m_buffer.lastIndexOf("[", endIndex);
    if (startIndex == -1 || startIndex >= endIndex)
        return;

    // Get last data instance
    QByteArray data = m_buffer.mid(startIndex + 1, endIndex - startIndex - 1);

    // Remove all data except incomplete data
    m_buffer = m_buffer.mid(endIndex + 1);

    Sensor::parseSerialData(data);
}

void Serial::tryToReconnectWhenErrorAppears(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::SerialPortError::NoError)
        return;

    Logger::crit(QString("Serial: Error occurred %1").arg(error));
    GlobalErrors::setError(GlobalErrors::SerialError);

    // Close the serial port if open
    if (m_serial->isOpen())
        m_serial->close();

    // Start the retry timer to reconnect every 5 seconds
    m_retryTimer.start();
}

Serial &Serial::instance()
{
    static Serial _instance{};
    return _instance;
}

void Serial::open()
{
    m_serial->setPortName("COM3");
    m_serial->setBaudRate(QSerialPort::Baud9600);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serial->open(QIODevice::ReadWrite)) {
        Logger::info("Serial opened");
    } else {
        Logger::crit(QString("Serial failed to open, error: %1").arg(m_serial->errorString()));
        GlobalErrors::setError(GlobalErrors::SerialError);
    }
}

void Serial::close()
{
    if (m_serial->isOpen())
        m_serial->close();
    Logger::info("SerialPort closed");
}

void Serial::sendData(QString data)
{
    data.append(";");

    auto succ = m_serial->write(data.toUtf8());

    if (succ == -1)
        GlobalErrors::setError(GlobalErrors::SerialSendError);
    else
        GlobalErrors::removeError(GlobalErrors::SerialSendError);
}
