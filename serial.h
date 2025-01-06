#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>
#include <QDateTime>
#include <QSerialPort>
#include <QTimer>

/**
 * @brief The Serial class represents a serial communication interface.
 * 
 * This class provides methods to open and close the serial port, send data,
 * and handle incoming data. It also includes functionality to handle reconnection
 * attempts in case of errors.
 */
class Serial : public QObject
/**
 * @brief The Serial class represents a serial communication interface.
 * 
 * This class provides methods to open and close the serial port, send data, and handle incoming data.
 * It is implemented as a singleton to ensure that only one instance of the Serial class exists.
 * 
 * @note This class is non-copyable and non-movable.
 */
{
    Q_OBJECT
public:
    Serial(const Serial&) = delete;
    Serial& operator=(const Serial &) = delete;
    Serial(Serial &&) = delete;
    Serial & operator=(Serial &&) = delete;
    ~Serial() = default;
    static Serial& instance();

    /**
     * @brief Opens the serial port.
     */
    void open();

    /**
     * @brief Closes the serial port.
     */
    void close();   

    /**
     * @brief Sends data over the serial port to arduino.
     * 
     * @param data The data to be sent over the serial port.
     */
    void sendData(const QString& data);

private:
    /**
     * @brief Private constructor.
     * 
     * @param parent The parent QObject.
     */
    explicit Serial(QObject *parent = nullptr);

    /**
     * @brief Parses the incoming data from sensors.
     */
    void parseData();

    QSerialPort *m_serial = nullptr; /**< The QSerialPort object for serial communication. */
    QByteArray m_buffer; /**< The buffer to store incoming data. */
    QTimer m_retryTimer; /**< The timer to handle reconnection attempts. */
    static constexpr int WAIT_TIME_MS = 2000; /**< The wait time in milliseconds for reconnection attempts. */

private slots:
    /**
     * @brief Slot to read incoming data from the serial port.
     */
    void readData();

    /**
     * @brief Slot to handle reconnection attempts when a serial port error occurs.
     * 
     * @param error The serial port error.
     */
    void tryToReconnectWhenErrorAppears(QSerialPort::SerialPortError error);

};

#endif // SERIAL_H
