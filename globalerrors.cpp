#include "globalerrors.h"

#include "logger.h"

// Database errors
const QString GlobalErrors::DB_ERROR = "Database error occurred";
const QString GlobalErrors::DB_GLOBAL_TARGET_K_LOAD_FAILED = "Database global target_k load failed, value is empty.";
const QString GlobalErrors::DB_GLOBAL_SERIAL_DATA_TIME_LOAD_FAILED = "Database global serial data time load failed, value is empty.";
const QString GlobalErrors::DB_GLOBAL_STATE_MACHINE_TICK_LOAD_FAILED = "Database global state machine tick load failed, value is empty.";
const QString GlobalErrors::DB_GLOBAL_STERILIZATION_TEMP_LOAD_FAILED = "Database global sterilization temp load failed, value is empty.";
const QString GlobalErrors::DB_GLOBAL_PASTERIZATION_TEMP_LOAD_FAILED = "Database global pasterization temp load failed, value is empty.";

// Serial errors
const QString GlobalErrors::SERIAL_ERROR = "Serial error occurred";
const QString GlobalErrors::OLD_DATA_ERROR = "Serial data is old";
const QString GlobalErrors::SERIAL_SEND_ERROR = "Failed to send serial data";

void GlobalErrors::setError(Error error)
{
    if (errors.testFlag(error))
        return;

    errors |= error;
    Logger::crit(QString("Global error occured: %1").arg(error));
}

void GlobalErrors::removeError(Error error)
{
    if (!errors.testFlag(error))
        return;

    errors ^= error;
    Logger::debug(QString("Global error resolved: %1").arg(error));
}

GlobalErrors::Errors GlobalErrors::getErrors()
{
    return errors;
}

QVector<QString> GlobalErrors::getErrorsString()
{
    QVector<QString> err;

    // Db error
    if (errors.testFlag(Error::DbError)) err.push_back(DB_ERROR);
    if (errors.testFlag(Error::DbSerialDataTimeError)) err.push_back(DB_GLOBAL_SERIAL_DATA_TIME_LOAD_FAILED);
    if (errors.testFlag(Error::DbStateMachineTickError)) err.push_back(DB_GLOBAL_STATE_MACHINE_TICK_LOAD_FAILED);
    if (errors.testFlag(Error::DbSterilizationTempError)) err.push_back(DB_GLOBAL_STERILIZATION_TEMP_LOAD_FAILED);
    if (errors.testFlag(Error::DbPasterizationTempError)) err.push_back(DB_GLOBAL_PASTERIZATION_TEMP_LOAD_FAILED);

    // Serial errors
    if (errors.testFlag(Error::SerialError)) err.push_back(SERIAL_ERROR);
    if (errors.testFlag(Error::OldDataError)) err.push_back(OLD_DATA_ERROR);
    if (errors.testFlag(Error::SerialSendError)) err.push_back(SERIAL_SEND_ERROR);

    return err;
}
