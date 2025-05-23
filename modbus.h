#ifndef MODBUS_H
#define MODBUS_H

#include <QObject>
#include <QModbusTcpClient>
#include <QModbusDataUnit>
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QQueue>
#include <QPair>

class Modbus : public QObject
{
    Q_OBJECT

public:
    static Modbus &instance();
    ~Modbus();

    static qint64 lastDataTime;    

    void connectToServer(const QString &ip, int port);
    void readInputRegisters();
    void writeSingleCoil(int coilAddress, bool value);

private:
    explicit Modbus(QObject *parent = nullptr);

    QModbusTcpClient *modbusClient = nullptr;
    QTimer retryTimer; /**< The timer to handle reconnection attempts. */
    static constexpr int WAIT_TIME_MS = 2000; /**< The wait time in milliseconds for reconnection attempts. */
    static constexpr int READ_INTERVAL_MS = 1000; /**< The wait time in milliseconds for reconnection attempts. */
    void attemptReconnect();

    QQueue<QPair<int, bool>> coilWriteQueue;
    QTimer writeQueueTimer;
    bool isWriting = false;
    static constexpr int WRITE_DELAY_MS = 100; // Adjust based on device requirements
    void processWriteQueue();

private slots:
    void onErrorOccurred(QModbusDevice::Error error);
    void onStateChanged(QModbusDevice::State state);
};

#endif // MODBUS_H
