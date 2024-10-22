#include "processlog.h"

#include "logger.h"

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
