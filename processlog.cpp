#include "processlog.h"

#include "logger.h"

ProcessLog::ProcessLog()
{
    Logger::crit("ProcessLog: Entered empty process log (this shouldn't happen)");
}


int ProcessLog::getId()
{
    return id;
}

QString ProcessLog::getName()
{
    return name;
}

QList<StateMachineValues> ProcessLog::getLogs()
{
    return logs;
}


int ProcessLog::saveProcessLog()
{
    // if (id == -1) {
    //     id = DbManager::instance().createProcess(name, info);
    //     return id;
    // } else {
    //     return DbManager::instance().updateProcess(id, info);
    // }
    return 0;
}
