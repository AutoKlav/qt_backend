#ifndef GRPCSERVER_H
#define GRPCSERVER_H

#include <QObject>
#include <memory>

/**
 * @brief The GRpcServer class represents a gRPC server.
 * 
 * This class is responsible for handling gRPC server operations.
 * It provides functionality to start and stop the server, as well as
 * perform other server-related tasks.
 */
class GRpcServer : public QObject
{
    Q_OBJECT
public:
    explicit GRpcServer(QObject *parent = nullptr);
    ~GRpcServer();

public slots:
    /**
     * @brief Shutdowns the gRPC server.
     * 
     * This slot is used to gracefully shutdown the gRPC server.
     * It stops the server and performs any necessary cleanup operations.
     */
    void shutdown();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

#endif // GRPCSERVER_H
