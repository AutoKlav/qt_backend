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
    QSqlQuery query("SELECT * FROM Globals", m_db);

    if (!query.exec()) {
        Logger::crit("Database: Unable to load globals");
        Logger::crit(QString("SQL error: %1").arg(query.lastError().text()));
        GlobalErrors::setError(GlobalErrors::DbError);
    }

    while (query.next()) {
        auto name = query.value(0).toString();
        auto value = query.value(1).toString();

        Globals::setVariable(name, value);
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

void DbManager::loadAnalogSensors()
{
    // Get number of sensors
    QSqlQuery count_query("SELECT COUNT(*) FROM AnalogSensor", m_db);

    // Resize Sensor::sensors to number of sensors
    if (count_query.exec() && count_query.next()) {
        Sensor::analogSensors.reserve(count_query.value(0).toInt());
    }

    QSqlQuery query("SELECT * FROM AnalogSensor", m_db);
    while (query.next()) {
        auto id = query.value(0).toUInt();
        auto alias = query.value(1).toString(); // alias is not used anywhere, just provides descriptions for virtual arduino pins
        auto minValue = query.value(2).toDouble();
        auto maxValue = query.value(3).toDouble();

        Sensor::analogSensors.append(Sensor(id, minValue, maxValue));
        Sensor::mapAnalogSensor.insert(id, &Sensor::analogSensors.last());
    }
}

void DbManager::loadDigitalSensors()
{
    // Get number of sensors
    QSqlQuery count_query("SELECT COUNT(*) FROM DigitalSensor", m_db);

    // Resize Sensor::sensors to number of sensors
    if (count_query.exec() && count_query.next()) {
        Sensor::digitalSensors.reserve(count_query.value(0).toInt());
    }

    QSqlQuery query("SELECT * FROM DigitalSensor", m_db);
    while (query.next()) {
        auto id = query.value(0).toUInt();
        auto alias = query.value(1).toString(); // alias is not used anywhere, just provides descriptions for virtual arduino pins

        Sensor::digitalSensors.append(Sensor(id));
        Sensor::mapDigitalSensor.insert(id, &Sensor::digitalSensors.last());
    }
}

bool DbManager::updateAnalogSensor(uint id, double newMinValue, double newMaxValue)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE AnalogSensor SET minValue = :minValue, maxValue = :maxValue WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":minValue", newMinValue);
    query.bindValue(":maxValue", newMaxValue);

    if (!query.exec()) {
        Logger::crit(QString("Database: Unable to update sensor %1").arg(id));
        Logger::crit(QString("SQL error: %1").arg(query.lastError().text()));
        return false;
    }

    Logger::info(QString("Database: Update sensor %1").arg(id));
    return true;
}

QList<ProcessRow> DbManager::getAllProcessesOrderedDesc()
{
    QSqlQuery query("SELECT process.id as id, process.batchLTO, process.productName, process.productQuantity, processStart, targetF, processLength, Bacteria.id as bacteriaId, Bacteria.name as bacteriaName, Bacteria.description as bacteriaDescription, d0, z, targetHeatingTime, targetCoolingTime FROM Process LEFT JOIN Bacteria ON Process.bacteriaId = Bacteria.id ORDER BY Process.processStart DESC", m_db);
    QList<ProcessRow> processes;
    while (query.next()) {
        auto id = query.value(0).toInt();
        auto batchLTO = query.value(1).toString();
        auto productName = query.value(2).toString();
        auto productQuantity = query.value(3).toString();
        auto processStart = query.value(4).toString();
        auto targetF = query.value(5).toString();
        auto processLength = query.value(6).toString();
        auto bacteriaId = query.value(7).toInt();
        auto bacteriaName = query.value(8).toString();
        auto bacteriaDescription = query.value(9).toString();
        auto d0 = query.value(10).toDouble();
        auto z = query.value(11).toDouble();
        auto targetHeatingTime = query.value(12).toString();
        auto targetCoolingTime = query.value(13).toString();

        const Bacteria bacteria = {
            .id = bacteriaId,
            .name = bacteriaName,
            .description = bacteriaDescription,
            .d0 = d0,
            .z = z
        };

        ProcessRow info;
        info.id = id;
        info.batchLTO = batchLTO;
        info.productName = productName;
        info.productQuantity = productQuantity;
        info.processStart = processStart;
        info.targetF = targetF;
        info.processLength = processLength;
        info.bacteria = bacteria;
        info.targetHeatingTime = targetHeatingTime;
        info.targetCoolingTime = targetCoolingTime;

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
        auto expansionTemp = query.value(2).toDouble();
        auto heaterTemp = query.value(3).toDouble();
        auto tankTemp = query.value(4).toDouble();
        auto tempK = query.value(5).toDouble();
        auto tankWaterLevel = query.value(6).toDouble();
        auto pressure = query.value(7).toDouble();
        auto steamPressure = query.value(8).toDouble();
        auto doorClosed = query.value(9).toDouble();
        auto burnerFault = query.value(10).toDouble();
        auto waterShortage = query.value(11).toDouble();
        auto dTemp = query.value(12).toDouble();
        auto state = query.value(13).toInt();
        auto Dr = query.value(14).toDouble();
        auto Fr = query.value(15).toDouble();
        auto r = query.value(16).toDouble();
        auto sumFr = query.value(17).toDouble();
        auto sumr = query.value(18).toDouble();
        auto timestamp = query.value(19).toString();

        ProcessLogInfoRow log;
        log.processId = id;
        log.temp = temp;
        log.expansionTemp = expansionTemp;
        log.heaterTemp = heaterTemp;
        log.tankTemp = tankTemp;
        log.tempK = tempK;
        log.tankWaterLevel = tankWaterLevel;
        log.pressure = pressure;
        log.steamPressure = steamPressure;
        log.doorClosed = doorClosed;
        log.burnerFault = burnerFault;
        log.waterShortage = waterShortage;
        log.dTemp = dTemp;
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
    query.prepare("INSERT INTO Process (bacteriaId, name, batchLTO, productName, productQuantity, processStart, targetF, targetHeatingTime, targetCoolingTime, processLength) "
                  "VALUES (:bacteriaId, :name, :batchLTO, :productName, :productQuantity, :processStart, :targetF, :targetHeatingTime, :targetCoolingTime, :processLength)");
    query.bindValue(":bacteriaId", info.bacteria.id);
    query.bindValue(":name", name);
    query.bindValue(":batchLTO", info.batchLTO);
    query.bindValue(":productName", info.productName);
    query.bindValue(":productQuantity", info.productQuantity);
    query.bindValue(":processStart", info.processStart);
    query.bindValue(":targetF", info.targetF);    
    query.bindValue(":targetHeatingTime", info.targetHeatingTime);
    query.bindValue(":targetCoolingTime", info.targetCoolingTime);
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
    query.prepare(
        "UPDATE Process SET productName = :productName, productQuantity = :productQuantity, "
        "processLength = :processLength, targetHeatingTime = :targetHeatingTime, "
        "targetCoolingTime = :targetCoolingTime WHERE id = :id"
        );
    query.bindValue(":id", id);
    query.bindValue(":productName", info.productName);
    query.bindValue(":productQuantity", info.productQuantity);
    query.bindValue(":processLength", info.processLength);
    query.bindValue(":targetHeatingTime", info.targetHeatingTime);
    query.bindValue(":targetCoolingTime", info.targetCoolingTime);

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

    // Must be checked since any provided literal is true in SQL
    QList<QString> allowedColumns = {"id", "name", "productName", "productQuantity", "bacteria",
                                 "description", "processStart", "targetHeatingTime", "targetCoolingTime" };
    if (!allowedColumns.contains(columnName)) {
        Logger::crit("Invalid column name: " + columnName);
        return QList<QString>();
    }

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

QMap<QString, QList<QString>> DbManager::getFilteredTargetFAndProcessLengthValues(QString productName, QString productQuantity)
{
    QMap<QString, QList<QString>> resultMap; // This will hold targetF and processLength as keys with their respective lists

    QSqlQuery query(m_db);

    // Correct the SQL query with named parameters
    QString queryStr = "SELECT DISTINCT targetF, processLength, targetHeatingTime, targetCoolingTime FROM Process WHERE productName LIKE :productName AND productQuantity LIKE :productQuantity ORDER BY id DESC";
    query.prepare(queryStr);

    // Bind the parameters safely with % wildcards for LIKE
    query.bindValue(":productName", productName);
    query.bindValue(":productQuantity", productQuantity);

    // Guard clause for query execution
    if (!query.exec()) {
        Logger::crit(query.lastError().text());
        Logger::crit("Query: " + query.executedQuery());
        return resultMap; // Return an empty map
    }

    // Initialize the lists for targetF and processLength
    QList<QString> targetFList;
    QList<QString> processLengthList;

    // Iterate over the result set
    while (query.next()) {
        QString targetF = query.value(0).toString();
        QString processLength = query.value(1).toString();

        // Avoid duplicates if necessary
        if (!targetFList.contains(targetF)) {
            targetFList.append(targetF);
        }

        if (!processLengthList.contains(processLength)) {
            processLengthList.append(processLength);
        }
    }

    // Store the results in the map
    resultMap["targetF"] = targetFList;
    resultMap["processLength"] = processLengthList;

    return resultMap;
}

int DbManager::createProcessType(ProcessType processType)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO ProcessType (name, type, customTemp, finishTemp, maintainTemp) "
                  "VALUES (:name, :type, :customTemp, :finishTemp, :maintainTemp)");
    query.bindValue(":name", processType.name);
    query.bindValue(":type", processType.type);
    query.bindValue(":customTemp", processType.customTemp);
    query.bindValue(":finishTemp", processType.finishTemp);    
    query.bindValue(":maintainTemp", processType.maintainTemp);

    if (!query.exec()) {
        Logger::crit(QString("Database: Unable to create process type %1").arg(processType.name));
        Logger::crit(QString("SQL error: %1").arg(query.lastError().text()));
        return -1;
    }

    Logger::info(QString("Database: Create process type %1").arg(processType.name));
    return query.lastInsertId().toInt();
}

int DbManager::createBacteria(Bacteria bacteria)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO Bacteria (name, description, d0, z, dateCreated) "
                  "VALUES (:name, :description, :d0, :z, :dateCreated)");
    query.bindValue(":name", bacteria.name);
    query.bindValue(":description", bacteria.description);
    query.bindValue(":d0", bacteria.d0);
    query.bindValue(":z", bacteria.z);
    query.bindValue(":dateCreated", QDateTime::currentDateTime());

    if (!query.exec()) {
        Logger::crit(QString("Database: Unable to create process type %1").arg(bacteria.name));
        Logger::crit(QString("SQL error: %1").arg(query.lastError().text()));
        return -1;
    }

    Logger::info(QString("Database: Create process type %1").arg(bacteria.name));
    return query.lastInsertId().toInt();
}

QList<Bacteria> DbManager::getBacteria()
{
    QSqlQuery query("SELECT * FROM Bacteria", m_db);
    QList<Bacteria> bacterias;
    while (query.next()) {
        auto id = query.value(0).toInt();
        auto name = query.value(1).toString();
        auto description = query.value(2).toString();
        auto d0 = query.value(3).toDouble();
        auto z = query.value(4).toDouble();

        bacterias.append({id, name, description, d0, z});
    }

    return bacterias;
}

int DbManager::deleteProcessType(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM ProcessType WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        Logger::crit(QString("Database: Unable to delete process type %1").arg(id));
        Logger::crit(QString("SQL error: %1").arg(query.lastError().text()));
        return -1;
    }

    Logger::info(QString("Database: Delete process type %1").arg(id));
    return query.numRowsAffected();
}

int DbManager::createProcessLog(int processId)
{
    StateMachine &stateMachine = StateMachine::instance();
    auto currentState = QString::number(stateMachine.getState());

    auto values = stateMachine.getValues();

    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO ProcessLog (processId, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, "
        "doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr, timestamp) "
        "VALUES (:processId, :temp, :expansionTemp, :heaterTemp, :tankTemp, :tempK, :tankWaterLevel, :pressure, :steamPressure, "
        ":doorClosed, :burnerFault, :waterShortage, :dTemp, :state, :Dr, :Fr, :r, :sumFr, :sumr, :timestamp)");

    query.bindValue(":processId", processId);
    query.bindValue(":temp", values.temp);
    query.bindValue(":expansionTemp", values.expansionTemp);
    query.bindValue(":heaterTemp", values.heaterTemp);
    query.bindValue(":tankTemp", values.tankTemp);
    query.bindValue(":tempK", values.tempK);
    query.bindValue(":tankWaterLevel", values.tankWaterLevel);
    query.bindValue(":pressure", values.pressure);
    query.bindValue(":steamPressure", values.steamPressure);
    query.bindValue(":doorClosed", values.doorClosed);
    query.bindValue(":burnerFault", values.burnerFault);
    query.bindValue(":waterShortage", values.waterShortage);
    query.bindValue(":dTemp", values.dTemp);
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

QStringList DbManager::getProcessesNames()
{
    QSqlQuery query("SELECT DISTINCT name FROM Process", m_db);
    QStringList names;
    while (query.next()) {
        names.append(query.value(0).toString());
    }
    return names;
}

QList<ProcessType> DbManager::getProcessTypes()
{
    QSqlQuery query("SELECT * FROM ProcessType", m_db);
    QList<ProcessType> types;
    while (query.next()) {
        auto id = query.value(0).toInt();
        auto name = query.value(1).toString();
        auto type = query.value(2).toString();
        auto customTemp = query.value(3).toDouble();
        auto finishTemp = query.value(4).toDouble();
        auto maintainTemp = query.value(5).toDouble();

        types.append({id, name, type, customTemp, finishTemp, maintainTemp});
    }

    return types;
}

DbManager& DbManager::instance()
{
    thread_local static DbManager _instance{};
    return _instance;
}
