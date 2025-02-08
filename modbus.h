#ifndef MODBUS_H
#define MODBUS_H

#include <QObject>
#include <QModbusTcpClient>
#include <QModbusDataUnit>
#include <QTimer>
#include <QDebug>

class Modbus : public QObject
{
    Q_OBJECT

public:
    static Modbus &instance();
    ~Modbus();

    void connectToServer(const QString &ip, int port);
    void readInputRegisters();
    void writeMultipleCoils();

private:
    explicit Modbus(QObject *parent = nullptr);

    QModbusTcpClient *modbusClient = nullptr;
    QTimer retryTimer; /**< The timer to handle reconnection attempts. */
    static constexpr int WAIT_TIME_MS = 2000; /**< The wait time in milliseconds for reconnection attempts. */

private slots:
    void onErrorOccurred(QModbusDevice::Error error);
    void onStateChanged(QModbusDevice::State state);
};

#endif // MODBUS_H
