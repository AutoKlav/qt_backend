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
    void loadAnalogSensors();
    void loadDigitalSensors();
    bool updateAnalogSensor(uint id, double newMinValue, double newMaxValue);

    // Process
    QList<ProcessRow> getAllProcessesOrderedDesc();
    QList<QString> getDistinctProcessValues(QString columnName);
    QMap<QString, QList<QString>> getFilteredTargetFAndProcessLengthValues(QString productName, QString productQuantity);
    QList<ProcessType> getProcessTypes();

    int createProcessType(ProcessType processType);
    int deleteProcessType(int id);
    int createProcess(QString name, ProcessInfo info);
    bool updateProcess(int id, ProcessInfo info);

    // Bacteria
    int createBacteria(Bacteria bacteria);
    QList<Bacteria> getBacteria();

    // ProcessLog
    QList<ProcessLogInfoRow> getAllProcessLogs(int processId);
    int createProcessLog(int processId);

    QStringList getProcessesNames();

    static DbManager& instance();

private:
    DbManager();

    QString loadGlobal(QString name);

    QSqlDatabase m_db;

};

#endif // DBMANAGER_H
