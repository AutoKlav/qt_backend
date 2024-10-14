#ifndef GRPCSERVER_H
#define GRPCSERVER_H

#include <QObject>
#include <memory>

class GRpcServer : public QObject
{
    Q_OBJECT
public:
    explicit GRpcServer(QObject *parent = nullptr);
    ~GRpcServer();

public slots:
    void shutdown();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

#endif // GRPCSERVER_H
