#include "globalerrors.h"

#include "logger.h"

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

    if (errors.testFlag(Error::DbError)) err.push_back("Database error occured");
    if (errors.testFlag(Error::SerialError)) err.push_back("Serial error occured");
    if (errors.testFlag(Error::OldDataError)) err.push_back("Serial data is old");
    if (errors.testFlag(Error::SerialSendError)) err.push_back("Failed to send serial data");

    return err;
}
