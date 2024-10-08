#include <QCoreApplication>

#include "master.h"
#include "grpcserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

//    qInstallMessageHandler(Logger::messageHandler);

    Master::instance();

    GRpcServer grpcServer;

    // On program interupt, stop the grpc server
    QObject::connect(&a, &QCoreApplication::aboutToQuit, &grpcServer, &GRpcServer::shutdown);

    return a.exec();
}
