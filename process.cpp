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

QList<ProcessRow> Process::getAllProcesses() {
    auto processes = DbManager::instance().getAllProcessesOrderedDesc();
    return processes;
}

QString Process::getName()
{
    return name;
}

int Process::getId()
{
    return id;
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

QList<QString> Process::getFilteredProcessValues(QString columnName)
{
    QList<QString> filteredValues = DbManager::instance().getDistinctProcessValues(columnName);
    return filteredValues;
}

QMap<QString, QList<QString>> Process::getFilteredTargetFAndProcessLengthValues(QString productName, QString productQuantity)
{
    QMap<QString, QList<QString>> map = DbManager::instance().getFilteredTargetFAndProcessLenghtValues(productName, productQuantity);
    return map;
}

QList<ProcessType> Process::getProcessTypes()
{
    QList<ProcessType> types = DbManager::instance().getProcessTypes();
    return types;

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
