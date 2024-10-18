#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "processlog.h"
#include "process.h"

/*
 * Globals:
 * name     string
 * value    string
 *
 * Sensor:
 * name     string
 * pin      string
 * minValue double
 * maxValue double
 */

class DbManager
{
public:
    struct ProcessFilters {
        QString name;
        QString minDate, maxDate;
    };

    DbManager(const DbManager&) = delete;
    DbManager& operator=(const DbManager &) = delete;
    DbManager(DbManager &&) = delete;
    DbManager & operator=(DbManager &&) = delete;

    // Globals
    void loadGlobals();
    bool updateGlobal(QString name, QString value);

    // Sensors
    void loadSensors();
    bool updateSensor(QString name, double newMinValue, double newMaxValue);

    // Process
    int createProcess(QString name, ProcessInfo info);
    bool updateProcess(int id, ProcessInfo info);

    // ProcessLog
    ProcessLog getProcessLog(int id);
    ProcessLog getProcessLog(QString name);
    int createProcessLog(QString name, ProcessInfo info);

    QList<ProcessRow> searchProcesses(ProcessFilters filters);
    QStringList getProcessesNames();

    static DbManager& instance();

private:
    DbManager();

    QString loadGlobal(QString name);

    QSqlDatabase m_db;

};

#endif // DBMANAGER_H
