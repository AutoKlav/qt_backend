#include "serial.h"

#include "sensor.h"
#include "logger.h"
#include "globalerrors.h"

Serial::Serial(QObject *parent)
    : QObject{parent}, m_serial(new QSerialPort(this))
{
    // Connect read data slot
    connect(m_serial, &QSerialPort::readyRead, this, &Serial::readData);
    // On port disconnect or error try reconnecting
    connect(m_serial, &QSerialPort::errorOccurred, this, &Serial::reconnect);
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
    int endIndex = m_buffer.lastIndexOf(";");
    if (endIndex == -1)
        return;

    // Find beginning of last data instance
    int startIndex = m_buffer.indexOf(";");
    if (startIndex == -1)
        return;

    if (startIndex == endIndex)
        return;

    // Get last data instance
    QByteArray data = m_buffer.mid(startIndex + 1, endIndex - startIndex - 2);
    // Remove all data except incomplete data
    m_buffer = m_buffer.mid(endIndex);

    Sensor::parseSerialData(data);
}

// TODO: Handle if Arduino is disconnected (stop program from crashing, retry every x seconds)
void Serial::reconnect(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::SerialPortError::NoError)
        return;

    Logger::crit(QString("Serial: Error occured %1").arg(error));
    GlobalErrors::setError(GlobalErrors::SerialError);

    if (m_serial->isOpen())
        m_serial->close();

    if (m_serial->open(QIODevice::ReadWrite))
        GlobalErrors::removeError(GlobalErrors::SerialError);
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
