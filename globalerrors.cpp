#include "globalerrors.h"

#include "logger.h"

const QString GlobalErrors::DB_ERROR = "Database error occurred";
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

    if (errors.testFlag(Error::DbError)) err.push_back(DB_ERROR);
    if (errors.testFlag(Error::SerialError)) err.push_back(SERIAL_ERROR);
    if (errors.testFlag(Error::OldDataError)) err.push_back(OLD_DATA_ERROR);
    if (errors.testFlag(Error::SerialSendError)) err.push_back(SERIAL_SEND_ERROR);

    return err;
}
