#ifndef GRPCSERVER_H
#define GRPCSERVER_H

#include <QObject>

#include <grpcpp/grpcpp.h>
#include <google/protobuf/text_format.h>

#include "autoklav.pb.h"
#include "autoklav.grpc.pb.h"

// Return status
using grpc::Status;

// GRPC stuff
using grpc::ServerContext;
using grpc::Server;

class AutoklavServiceImplementation final : public Autoklav::Autoklav::Service
{
    Status getStatus(ServerContext* context,
                     const Autoklav::Empty* request,
                     Autoklav::Status* replay);

    Status getVariables(ServerContext* context,
                        const Autoklav::Empty* request,
                        Autoklav::Variables* replay);

    Status setVariable(ServerContext* context,
                       const Autoklav::SetVariable* request,
                       Autoklav::Status* replay);

    Status startProcess(ServerContext* context,
                        const Autoklav::StartProcessRequest* request,
                        Autoklav::Status* replay);

    Status stopProcess(ServerContext* context,
                       const Autoklav::Empty* request,
                       Autoklav::Status* replay);

    Status getSensorValues(ServerContext* context,
                           const Autoklav::Empty* request,
                           Autoklav::SensorValues* replay);

    Status getStateMachineValues(ServerContext* context,
                                 const Autoklav::Empty* request,
                                 Autoklav::StateMachineValues* replay);

    void setStatusReply(Autoklav::Status* replay, int code);
};

class GRpcServer : public QObject
{
    Q_OBJECT
public:
    explicit GRpcServer(QObject *parent = nullptr);

public slots:
    void shutdown();

private:
    AutoklavServiceImplementation service;
    std::shared_ptr<Server> server;
};

#endif // GRPCSERVER_H
