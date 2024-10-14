#include "dbmanager.h"

#include <QThread>

#include "sensor.h"
#include "logger.h"
#include "globals.h"
#include "globalerrors.h"

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
    // TODO: Don't set if loadGlobal() returns empty string
    Globals::targetK = loadGlobal("targetK").toDouble();
    Globals::serialDataTime = loadGlobal("serialDataTime").toInt();
    Globals::stateMachineTick = loadGlobal("stateMachineTick").toInt();
    Globals::sterilizationTemp = loadGlobal("sterilizationTemp").toDouble();
    Globals::pasterizationTemp = loadGlobal("pasterizationTemp").toDouble();
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

ProcessLog DbManager::getProcessLog(int id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM ProcessLog WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        auto id = query.value(0).toInt();
        auto name = query.value(1).toString();
        auto productName = query.value(2).toString();
        auto productQuantity = query.value(3).toString();
        auto bacteria = query.value(4).toString();
        auto description = query.value(5).toString();
        auto processStart = query.value(6).toString();
        auto processLength = query.value(7).toString();

        return ProcessLog(id, name, {productName, productQuantity, bacteria, description, processStart, processLength});
    } else {
        Logger::crit(QString("Database: Unable to fetch process log %1").arg(id));
        Logger::crit(QString("SQL error: %1").arg(query.lastError().text()));
        return ProcessLog();
    }
}

ProcessLog DbManager::getProcessLog(QString name)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM ProcessLog WHERE name = :name");
    query.bindValue(":name", name);

    if (query.exec() && query.next()) {
        auto id = query.value(0).toInt();
        auto name = query.value(1).toString();
        auto productName = query.value(2).toString();
        auto productQuantity = query.value(3).toString();
        auto bacteria = query.value(4).toString();
        auto description = query.value(5).toString();
        auto processStart = query.value(6).toString();
        auto processLength = query.value(7).toString();

        return ProcessLog(id, name, {productName, productQuantity, bacteria, description, processStart, processLength});
    } else {
        Logger::crit(QString("Database: Unable to fetch process log %1").arg(name));
        Logger::crit(QString("SQL error: %1").arg(query.lastError().text()));
        return ProcessLog();
    }
}

int DbManager::createProcessLog(QString name, ProcessInfo info)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO ProcessLog (name, productName, productQuantity, bacteria, description, processStart, processLength) "
                  "VALUES (:name, :productName, :productQuantity, :bacteria, :description, :processStart, :processLength)");
    query.bindValue(":name", name);
    query.bindValue(":productName", info.productName);
    query.bindValue(":productQuantity", info.productQuantity);
    query.bindValue(":bacteria", info.bacteria);
    query.bindValue(":description", info.description);
    query.bindValue(":processStart", info.processStart);
    query.bindValue(":processLength", info.processLength);

    if (!query.exec()) {
        Logger::crit(QString("Database: Unable to create process log %1").arg(name));
        Logger::crit(QString("SQL error: %1").arg(query.lastError().text()));
        return -1;
    }

    Logger::info(QString("Database: Create process log %1").arg(name));
    return query.lastInsertId().toInt();
}

bool DbManager::updateProcessLog(int id, ProcessInfo info)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE ProcessLog SET productName = :productName, productQuantity = :productQuantity, bacteria = :bacteria, "
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

    Logger::info(QString("Database: Update process log %1").arg(id));
    return true;
}

QList<ProcessLogRow> DbManager::searchProcessLogs(ProcessLogFilters filters)
{
    QString queryStr = "SELECT * FROM ProcessLog WHERE 1";
    if (!filters.name.isEmpty()) {
        queryStr += " AND name LIKE '%" + filters.name + "%'";
    }
    if (!filters.minDate.isEmpty() && !filters.maxDate.isEmpty()) {
        queryStr += " AND processStart BETWEEN '" + filters.minDate + "' AND '" + filters.maxDate + "'";
    }

    QSqlQuery query(queryStr, m_db);
    QList<ProcessLogRow> rows;
    while (query.next()) {
        auto id = query.value(0).toInt();
        // auto name = query.value(1).toString();
        auto productName = query.value(2).toString();
        auto productQuantity = query.value(3).toString();
        auto bacteria = query.value(4).toString();
        auto description = query.value(5).toString();
        auto processStart = query.value(6).toString();
        auto processLength = query.value(7).toString();

        ProcessLogRow row = {
            {productName, productQuantity, bacteria, description, processStart, processLength}, id
        };
        rows.append(row);
    }
    return rows;
}

QStringList DbManager::getProcessLogNames()
{
    QSqlQuery query("SELECT DISTINCT name FROM ProcessLog", m_db);
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
