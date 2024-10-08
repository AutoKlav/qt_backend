#ifndef MASTER_H
#define MASTER_H

#include <QObject>

class Master : public QObject
{
    Q_OBJECT
public:
    Master(const Master&) = delete;
    Master& operator=(const Master &) = delete;
    Master(Master &&) = delete;
    Master & operator=(Master &&) = delete;
    ~Master() = default;

    static Master &instance();

private:
    explicit Master(QObject *parent = nullptr);

};

#endif // MASTER_H


/*
 * TODO:
 *
 * [x] parse data from serial
 * [x] sensor class /w map
 * [x] sensor data in db
 * [x] current sensor state struct
 * [ ] current state struct
 *
 *
 * StateMachine:
 * [x] start/stop
 * [x] QTimer
 * [x] stages
 * [x] current state machine struct
 * [ ] logging
 */
