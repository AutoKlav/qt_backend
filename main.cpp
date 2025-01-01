#include <QCoreApplication>

#include "master.h"
#include "grpcserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Master::instance();

    GRpcServer grpcServer;

    return a.exec();
}
