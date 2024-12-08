#include "dbmanager.h"

#include <QThread>

#include "sensor.h"
#include "logger.h"
#include "globals.h"
#include "globalerrors.h"
#include "statemachine.h"

DbManager::DbManager()
{
    // Get appData folder
    auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    m_db = QSqlDatabase::addDatabase("QSQLITE", QString::number((quint64)QThread::currentThread(), 16));
    m_db.setDatabaseName(path + "/db.sqlite");

    if (!m_db.open()) {
        auto error = m_db.lastError();
        Logger::crit("Database: Connection with database failed.");
        Logger::crit(QString("[%1]: %2").arg(error.nativeErrorCode(), error.text()));
        GlobalErrors::setError(GlobalErrors::DbError);
    }

    Logger::info("Database: ok");
}

QString DbManager::loadGlobal(QString name)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT value FROM Globals WHERE name = :name");
    query.bindValue(":name", name);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    } else {
        Logger::crit(QString("Database: Unable to fetch global %1").arg(name));
        Logger::crit(QString("SQL error: %1").arg(query.lastError().text()));
        return "";
    }
}

void DbManager::loadGlobals()
{
    QString serialDataTimeStr = loadGlobal("serialDataTime");
    if (!serialDataTimeStr.isEmpty()) {
        Globals::serialDataTime = serialDataTimeStr.toInt();
    }
    else {
        Logger::crit(GlobalErrors::DB_GLOBAL_SERIAL_DATA_TIME_LOAD_FAILED);
        GlobalErrors::setError(GlobalErrors::DbSerialDataTimeError);
    }

    QString stateMachineTickStr = loadGlobal("stateMachineTick");
    if (!stateMachineTickStr.isEmpty()) {
        Globals::stateMachineTick = stateMachineTickStr.toInt();
    }
    else {
        Logger::crit(GlobalErrors::DB_GLOBAL_STATE_MACHINE_TICK_LOAD_FAILED);
        GlobalErrors::setError(GlobalErrors::DbStateMachineTickError);
    }

    QString sterilizationTempStr = loadGlobal("sterilizationTemp");
    if (!sterilizationTempStr.isEmpty()) {
        Globals::sterilizationTemp = sterilizationTempStr.toDouble();
    }
    else {
        Logger::crit(GlobalErrors::DB_GLOBAL_STERILIZATION_TEMP_LOAD_FAILED);
        GlobalErrors::setError(GlobalErrors::DbSterilizationTempError);
    }

    QString pasterizationTempStr = loadGlobal("pasterizationTemp");
    if (!pasterizationTempStr.isEmpty()) {
        Globals::pasterizationTemp = pasterizationTempStr.toDouble();
    }
    else {
        Logger::crit(GlobalErrors::DB_GLOBAL_PASTERIZATION_TEMP_LOAD_FAILED);
        GlobalErrors::setError(GlobalErrors::DbPasterizationTempError);
    }
}

bool DbManager::updateGlobal(QString name, QString value)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE Globals SET value = :value WHERE name = :name");
    query.bindValue(":name", name);
    query.bindValue(":value", value);

    if (!query.exec()) {
        Logger::crit(QString("Database: Unable to update global %1 to %2").arg(name, value));
        Logger::crit(QString("SQL error: %1").arg(query.lastError().text()));
        return false;
    }

    Logger::info(QString("Database: Update global %1 to %2").arg(name, value));
    return true;
}

void DbManager::loadSensors()
{
    // Get number of sensors
    QSqlQuery count_query("SELECT COUNT(*) FROM Sensor", m_db);

    // Resize Sensor::sensors to number of sensors
    if (count_query.exec() && count_query.next()) {
        Sensor::sensors.reserve(count_query.value(0).toInt());
    }

    QSqlQuery query("SELECT * FROM Sensor", m_db);
    while (query.next()) {
        auto name = query.value(0).toString();
        auto pinName = query.value(1).toString();
        auto minValue = query.value(2).toDouble();
        auto maxValue = query.value(3).toDouble();

        Sensor::sensors.append(Sensor(name, pinName, minValue, maxValue));
        Sensor::mapName.insert(name, &Sensor::sensors.last());
        Sensor::mapPinName.insert(pinName, &Sensor::sensors.last());
    }
}

bool DbManager::updateSensor(QString name, double newMinValue, double newMaxValue)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE Sensor SET minValue = :minValue, maxValue = :maxValue WHERE name = :name");
    query.bindValue(":name", name);
    query.bindValue(":minValue", newMinValue);
    query.bindValue(":maxValue", newMaxValue);

    if (!query.exec()) {
        Logger::crit(QString("Database: Unable to update sensor %1").arg(name));
        Logger::crit(QString("SQL error: %1").arg(query.lastError().text()));
        return false;
    }

    Logger::info(QString("Database: Update sensor %1").arg(name));
    return true;
}

QList<ProcessRow> DbManager::getAllProcessesOrderedDesc()
{
    QSqlQuery query("SELECT * FROM Process ORDER BY processStart desc", m_db);
    QList<ProcessRow> processes;
    while (query.next()) {
        auto id = query.value(0).toInt();
        auto name = query.value(1).toString();
        auto productName = query.value(2).toString();
        auto productQuantity = query.value(3).toString();
        auto bacteria = query.value(4).toString();
        auto description = query.value(5).toString();
        auto processStart = query.value(6).toString();
        auto targetF = query.value(7).toString();
        auto processLength = query.value(8).toString();

        ProcessRow info;
        info.id = id;
        info.productName = productName;
        info.productQuantity = productQuantity;
        info.bacteria = bacteria;
        info.description = description;
        info.processStart = processStart;
        info.targetF = targetF;
        info.processLength = processLength;

        processes.append(info);
    }

    return processes;
}

QList<ProcessLogInfoRow> DbManager::getAllProcessLogs(int processId)
{
    QList<ProcessLogInfoRow> processLogs;

    QSqlQuery query(m_db);

    query.prepare("SELECT * FROM ProcessLog WHERE processId = :processId ORDER BY timestamp ASC");
    query.bindValue(":processId", processId);

    // Guard clause for query execution
    if (!query.exec()) {
        Logger::crit(query.lastError().text());
        Logger::crit("Query: " + query.executedQuery());
        return processLogs;
    }

    while (query.next()) {
        auto id = query.value(0).toInt();
        auto temp = query.value(1).toDouble();
        auto tempK = query.value(2).toDouble();
        auto dTemp = query.value(3).toDouble();
        auto pressure = query.value(4).toDouble();
        auto state = query.value(5).toInt();
        auto Dr = query.value(6).toDouble();
        auto Fr = query.value(7).toDouble();
        auto r = query.value(8).toDouble();
        auto sumFr = query.value(9).toDouble();
        auto sumr = query.value(10).toDouble();
        auto timestamp = query.value(11).toString();

        ProcessLogInfoRow log;
        log.processId = id;
        log.temp = temp;
        log.tempK = tempK;
        log.dTemp = dTemp;
        log.pressure = pressure;
        log.state = state;
        log.Dr = Dr;
        log.Fr = Fr;
        log.r = r;
        log.sumFr = sumFr;
        log.sumr = sumr;
        log.timestamp = timestamp;

        processLogs.append(log);
    }

    return processLogs;
}

int DbManager::createProcess(QString name, ProcessInfo info)
{    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO Process (name, productName, productQuantity, bacteria, description, processStart, targetF, processLength) "
                  "VALUES (:name, :productName, :productQuantity, :bacteria, :description, :processStart, :targetF, :processLength)");
    query.bindValue(":name", name);
    query.bindValue(":productName", info.productName);
    query.bindValue(":productQuantity", info.productQuantity);
    query.bindValue(":bacteria", info.bacteria);
    query.bindValue(":description", info.description);
    query.bindValue(":processStart", info.processStart);
    query.bindValue(":targetF", info.targetF);
    query.bindValue(":processLength", info.processLength);

    if (!query.exec()) {
        Logger::crit(QString("Database: Unable to create process log %1").arg(name));
        Logger::crit(QString("SQL error: %1").arg(query.lastError().text()));
        return -1;
    }

    Logger::info(QString("Database: Create process log %1").arg(name));
    return query.lastInsertId().toInt();
}

bool DbManager::updateProcess(int id, ProcessInfo info)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE Process SET productName = :productName, productQuantity = :productQuantity, bacteria = :bacteria, "
                  "description = :description, processLength = :processLength WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":productName", info.productName);
    query.bindValue(":productQuantity", info.productQuantity);
    query.bindValue(":bacteria", info.bacteria);
    query.bindValue(":description", info.description);
    query.bindValue(":processLength", info.processLength);

    if (!query.exec()) {
        Logger::crit(QString("Database: Unable to update process log %1").arg(id));
        Logger::crit(QString("SQL error: %1").arg(query.lastError().text()));
        return false;
    }

    Logger::info(QString("Database: Update process %1").arg(id));
    return true;
}

QList<QString> DbManager::getDistinctProcessValues(QString columnName)
{
    QList<QString> filteredValues;

    QSqlQuery query(m_db);

    // Directly concatenate the column name into the SQL query
    QString queryStr = QString("SELECT DISTINCT %1 FROM Process ORDER BY id DESC").arg(columnName);
    query.prepare(queryStr);

    // Guard clause for query execution
    if (!query.exec()) {
        Logger::crit(query.lastError().text());
        Logger::crit("Query: " + query.executedQuery());
        return QList<QString>();
    }

    while (query.next()) {
        auto response = query.value(0).toString();
        filteredValues.append(response);
    }

    return filteredValues;
}

int DbManager::createProcessLog(int processId)
{
    StateMachine &stateMachine = StateMachine::instance();
    auto currentState = QString::number(stateMachine.getState());

    auto values = stateMachine.getValues();

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO ProcessLog (processId, temp, tempK, dTemp, pressure, state, Dr, Fr, r, sumFr, sumr, timestamp) "
                  "VALUES (:processId, :temp, :tempK, :dTemp, :pressure, :state, :Dr, :Fr, :r, :sumFr, :sumr, :timestamp)");
    query.bindValue(":processId", processId);
    query.bindValue(":temp", values.temp);
    query.bindValue(":tempK", values.tempK);
    query.bindValue(":dTemp", values.dTemp);
    query.bindValue(":pressure", values.pressure);
    query.bindValue(":state", currentState);
    query.bindValue(":Dr", values.Dr);
    query.bindValue(":Fr", values.Fr);
    query.bindValue(":r", values.r);
    query.bindValue(":sumFr", values.sumFr);
    query.bindValue(":sumr", values.sumr);
    query.bindValue(":timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    if (!query.exec()) {
        Logger::crit(QString("Database: Unable to create process log"));
        Logger::crit(QString("SQL error: %1").arg(query.lastError().text()));
        return -1;
    }

    Logger::info(QString("Database: Created process log entry"));
    return query.lastInsertId().toInt();
}

QList<ProcessRow> DbManager::searchProcesses(ProcessFilters filters)
{
    QString queryStr = "SELECT * FROM Process WHERE 1";
    if (!filters.name.isEmpty()) {
        queryStr += " AND name LIKE '%" + filters.name + "%'";
    }
    if (!filters.minDate.isEmpty() && !filters.maxDate.isEmpty()) {
        queryStr += " AND processStart BETWEEN '" + filters.minDate + "' AND '" + filters.maxDate + "'";
    }

    QSqlQuery query(queryStr, m_db);
    QList<ProcessRow> rows;
    while (query.next()) {
        auto id = query.value(0).toInt();
        // auto name = query.value(1).toString();
        auto productName = query.value(2).toString();
        auto productQuantity = query.value(3).toString();
        auto bacteria = query.value(4).toString();
        auto description = query.value(5).toString();
        auto processStart = query.value(6).toString();
        auto processLength = query.value(7).toString();

        ProcessRow row = {
            {productName, productQuantity, bacteria, description, processStart, processLength}, id
        };
        rows.append(row);
    }
    return rows;
}

QStringList DbManager::getProcessesNames()
{
    QSqlQuery query("SELECT DISTINCT name FROM Process", m_db);
    QStringList names;
    while (query.next()) {
        names.append(query.value(0).toString());
    }
    return names;
}

DbManager& DbManager::instance()
{
    thread_local static DbManager _instance{};
    return _instance;
}
