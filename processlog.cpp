#include "processlog.h"

#include "logger.h"
#include "dbmanager.h"

ProcessLog::ProcessLog()
{
    Logger::crit("ProcessLog: Entered empty process log (this shouldn't happen)");
}

ProcessLog::ProcessLog(QString name, ProcessInfo info, QObject *parent)
    : QObject{parent}, name{name}, info{info}, file{QString("%1/logs/%2.txt").arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation), name), this}
{
    this->info.processStart = name;
    save();
    file.open(QFile::Append | QFile::Text);
}

ProcessLog::ProcessLog(int id, QString name, ProcessInfo info, QObject *parent)
    : QObject{parent}, id{id}, name{name}, info{info}, file{QString("logs/%1.txt").arg(name), this}
{
    loadLogs();
    file.open(QFile::Append | QFile::Text);
}

bool ProcessLog::appendLog(StateMachineValues log)
{
    logs.append(log);

    QTextStream out(&file);
    out << log.toString() << Qt::endl;
    out.flush();

    return true;
}

int ProcessLog::getId()
{
    return id;
}

QString ProcessLog::getName()
{
    return name;
}

ProcessInfo ProcessLog::getInfo()
{
    return info;
}

QList<StateMachineValues> ProcessLog::getLogs()
{
    return logs;
}

bool ProcessLog::setInfo(ProcessInfo newInfo)
{
    auto oldInfo = info;
    info = newInfo;

    if (!save()) {
        info = oldInfo;
        Logger::crit(QString("ProcessLog: Unable to update info of id=%1").arg(id));
        return false;
    }

    return true;
}

void ProcessLog::loadLogs()
{
    if (!file.exists()) {
        Logger::warn(QString("ProcessLog: File %1 doesn't exist").arg(name));
        return;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        Logger::warn(QString("ProcessLog: Couldn't load logs from file %1").arg(name));
        return;
    }

    QTextStream in(&file);
    QString line;
    while (in.readLineInto(&line)) {
        logs.append(StateMachineValues::parse(line));
    }

    file.close();
}

bool ProcessLog::save()
{
    if (id == -1) {
        id = DbManager::instance().createProcessLog(name, info);
        return id != -1;
    } else {
        return DbManager::instance().updateProcessLog(id, info);
    }
}
