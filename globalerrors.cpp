#include "globalerrors.h"

#include "logger.h"

// Database errors
const QString GlobalErrors::DB_ERROR = "Database error occurred";
const QString GlobalErrors::DB_GLOBALS = "Database failed to load globals.";

// Modbus errors
const QString GlobalErrors::MODBUS_ERROR = "Veza sa autoklavom nije uspjela, automatsko spajanje (30s)...";
const QString GlobalErrors::OLD_DATA_ERROR = "TCP data is old";

// Autoklav errors
const QString GlobalErrors::DOOR_CLOSED_ERROR = "Vrata nisu zatvorena!";
const QString GlobalErrors::BURNER_ERROR = "Plamenik nije uključen!";
const QString GlobalErrors::WATER_SHORTAGE_ERROR = "Nema dovoljno vode!";
const QString GlobalErrors::ALREADY_STARTED = "Program je već pokrenut!";

const QString GlobalErrors::MODBUS_WRITE_COIL_ERROR = "Greška prilikom uključivanja izlaznog senzora!";
const QString GlobalErrors::MODBUS_READ_REGISTER_ERROR = "Greška prilikom čitanja podataka!";

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
    if (errors.testFlag(Error::DbGlobalsError)) err.push_back(DB_GLOBALS);
    
    // Modbus errors
    if (errors.testFlag(Error::ModbusError)) err.push_back(MODBUS_ERROR);
    if (errors.testFlag(Error::OldDataError)) err.push_back(OLD_DATA_ERROR);

    // Autoklav errors
    if (errors.testFlag(Error::DoorClosedError)) err.push_back(DOOR_CLOSED_ERROR);
    if (errors.testFlag(Error::BurnerError)) err.push_back(BURNER_ERROR);
    if (errors.testFlag(Error::WaterShortageError)) err.push_back(WATER_SHORTAGE_ERROR);
    if (errors.testFlag(Error::AlreadyStarted)) err.push_back(ALREADY_STARTED);
    if (errors.testFlag(Error::ModbusWriteCoilError)) err.push_back(MODBUS_WRITE_COIL_ERROR);
    if (errors.testFlag(Error::ModbusReadRegisterError)) err.push_back(MODBUS_READ_REGISTER_ERROR);

    return err;
}
