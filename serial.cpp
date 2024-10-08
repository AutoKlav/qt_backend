#include "serial.h"

#include "sensor.h"
#include "logger.h"
#include "globalerrors.h"

Serial::Serial(QObject *parent)
    : QObject{parent}
{
    // Create serial port
    m_serial = std::make_unique<QSerialPort>("/dev/pts/1", this);
    m_serial->setBaudRate(QSerialPort::Baud9600);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    m_serial->open(QIODevice::ReadWrite);

    // Connect read data slot
    connect(m_serial.get(), &QSerialPort::readyRead, this, &Serial::readData);
    // On port disconnect or error try reconnecting
    connect(m_serial.get(), &QSerialPort::errorOccurred, this, &Serial::reconnect);
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
    int endIndex = m_buffer.lastIndexOf("#");
    if (endIndex == -1)
        return;

    // Find beging of last data instance
    int startIndex = m_buffer.lastIndexOf("Sensors;", endIndex);
    if (startIndex == -1)
        return;

    // Get last data instance
    QByteArray data = m_buffer.mid(startIndex, endIndex - startIndex + 1);
    // Remove all data except incomplete data
    m_buffer = m_buffer.mid(endIndex + 1);

    Sensor::parseSerialData(data);
}

void Serial::reconnect(QSerialPort::SerialPortError error)
{
    Logger::crit(QString("Serial: Error occured %1").arg(error));
    GlobalErrors::setError(GlobalErrors::SerialError);

    m_serial->close();
    if (m_serial->open(QIODevice::ReadWrite))
        GlobalErrors::removeError(GlobalErrors::SerialError);
}

Serial &Serial::instance()
{
    static Serial _instance{};
    return _instance;
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
