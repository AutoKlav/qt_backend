#include <QCoreApplication>

#include "logger.h"
#include "master.h"
#include "grpcserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qInstallMessageHandler(Logger::messageHandler);

    Master::instance();

    GRpcServer grpcServer;

    return a.exec();
}
