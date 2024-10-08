#include "mockserial.h"

#include "sensor.h"

#include <QDebug>

MockSerial::MockSerial(QObject *parent)
    : QObject{parent}
{
    timer = new QTimer();
    QObject::connect(timer, &QTimer::timeout, this, &MockSerial::readData);

    timer->start(5000);
}

void MockSerial::readData()
{
    // Read from file and return
    QFile file("data.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    QString line = in.readLine();
    file.close();

    Sensor::parseSerialData(line);
}

void MockSerial::sendData(QString data)
{
    qDebug() << "Serial:" << data;
}

MockSerial &MockSerial::instance()
{
    static MockSerial _instance{};
    return _instance;
}
