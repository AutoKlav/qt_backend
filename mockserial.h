#ifndef MOCKMockSerial_H
#define MOCKMockSerial_H

#include <QFile>
#include <QTimer>
#include <QString>
#include <QObject>
#include <QTextStream>

class MockSerial : public QObject
{
    Q_OBJECT
public:
    MockSerial(const MockSerial&) = delete;
    MockSerial& operator=(const MockSerial &) = delete;
    MockSerial(MockSerial &&) = delete;
    MockSerial & operator=(MockSerial &&) = delete;
    ~MockSerial() = default;

    static MockSerial &instance();

    void sendData(QString data);

public slots:
    void readData();

private:
    explicit MockSerial(QObject *parent = nullptr);

    QTimer *timer;
};

#endif // MOCKMockSerial_H
