#include <QCoreApplication>

#include "master.h"
#include "grpcserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

//    qInstallMessageHandler(Logger::messageHandler); // Enable logging to file

    Master::instance();

    GRpcServer grpcServer;

    return a.exec();
}
