#include <QCoreApplication>

#include "master.h"
#include "grpcserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

//    qInstallMessageHandler(Logger::messageHandler); // Enable logging to file

    Master::instance();

    auto server = std::make_unique<GRpcServer>();
    server->run();

    return a.exec();
}
