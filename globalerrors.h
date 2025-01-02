#ifndef GLOBALERRORS_H
#define GLOBALERRORS_H

#include <QFlags>

class GlobalErrors
{
public:
    enum Error {
        DbError = 0x1,
        SerialError = 0x2,
        DbSerialDataTimeError = 0x4,
        DbStateMachineTickError = 0x8,
        DbKError = 0x10,
        DbCoolingThresholdError = 0x20,
        DbExpansionTempError = 0x40,
        OldDataError = 0x80,
        SerialSendError = 0x100
    };
    Q_DECLARE_FLAGS(Errors, Error);

    // Db errors
    static const QString DB_ERROR;
    static const QString DB_GLOBAL_SERIAL_DATA_TIME_LOAD_FAILED;
    static const QString DB_GLOBAL_STATE_MACHINE_TICK_LOAD_FAILED;
    static const QString DB_GLOBAL_K_LOAD_FAILED;
    static const QString DB_GLOBAL_COOLING_THRESHOLD_LOAD_FAILED;
    static const QString DB_GLOBAL_EXPANSION_TEMP_LOAD_FAILED;
    
    // Serial errors
    static const QString SERIAL_ERROR;
    static const QString OLD_DATA_ERROR;
    static const QString SERIAL_SEND_ERROR;

    static void setError(Error error);
    static void removeError(Error error);
    static Errors getErrors();
    static QVector<QString> getErrorsString();

private:
    inline static Errors errors;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(GlobalErrors::Errors)

#endif // GLOBALERRORS_H
