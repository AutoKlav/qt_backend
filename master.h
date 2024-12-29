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