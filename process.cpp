#include "process.h"
#include "dbmanager.h"
#include "logger.h"

Process::Process()
{
    Logger::crit("Process: Entered empty process log (this shouldn't happen)");
}

Process::Process(QString name, ProcessInfo info, QObject *parent)
    : QObject{parent}, name{name}, info{info}
{
    this->info.processStart = name;
    saveProcess();
}

QString Process::getName()
{
    return name;
}

ProcessInfo Process::getInfo()
{
    return info;
}

bool Process::setInfo(ProcessInfo newInfo)
{
    auto oldInfo = info;
    info = newInfo;

    if (!saveProcess()) {
        info = oldInfo;
        Logger::crit(QString("ProcessLog: Unable to update info of id=%1").arg(id));
        return false;
    }

    return true;
}

int Process::saveProcess()
{
    if (id == -1) {
        id = DbManager::instance().createProcess(name, info);
        return id;
    } else {
        return DbManager::instance().updateProcess(id, info);
    }
}
