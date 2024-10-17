#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "processlog.h"

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
    struct ProcessLogFilters {
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

    /// Process
    int createProcess(QString name, ProcessInfo info);

    // ProcessLog
    ProcessLog getProcessLog(int id);
    ProcessLog getProcessLog(QString name);
    int createProcessLog(QString name, ProcessInfo info);
    bool updateProcessLog(int id, ProcessInfo info);

    QList<ProcessLogRow> searchProcessLogs(ProcessLogFilters filters);
    QStringList getProcessLogNames();

    static DbManager& instance();

private:
    DbManager();

    QString loadGlobal(QString name);

    QSqlDatabase m_db;

};

#endif // DBMANAGER_H
