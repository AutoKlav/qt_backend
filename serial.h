#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>
#include <QDateTime>
#include <QSerialPort>

class Serial : public QObject
{
    Q_OBJECT
public:
    Serial(const Serial&) = delete;
    Serial& operator=(const Serial &) = delete;
    Serial(Serial &&) = delete;
    Serial & operator=(Serial &&) = delete;
    ~Serial() = default;

    static Serial& instance();

    void open();
    void close();
    void sendData(QString data);

private:
    explicit Serial(QObject *parent = nullptr);

    void parseData();

    QSerialPort *m_serial = nullptr;
    QByteArray m_buffer;

private slots:
    void readData();
    void reconnect(QSerialPort::SerialPortError error);

};

#endif // SERIAL_H
