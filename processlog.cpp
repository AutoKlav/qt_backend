#include "processlog.h"

#include "logger.h"
#include "dbmanager.h"

ProcessLog::ProcessLog()
{
    Logger::crit("ProcessLog: Entered empty process log (this shouldn't happen)");
}

QString ProcessLog::getName()
{
    return name;
}

QList<StateMachineValues> ProcessLog::getLogs()
{
    return logs;
}

QList<ProcessLogInfoRow> ProcessLog::getAllProcessLogs(int processId)
{
    auto processLogs = DbManager::instance().getAllProcessLogs(processId);
    return processLogs;
}
