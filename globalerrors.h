#ifndef GLOBALERRORS_H
#define GLOBALERRORS_H

#include <QFlags>

class GlobalErrors
{
public:
    enum Error {
        DbError = 0x1,
        DbGlobalsError = 0x2,
        OldDataError = 0x4,     
        ModbusError = 0x8,  
        DoorClosedError = 0x10,
        BurnerError = 0x20,
        WaterShortageError = 0x40, 
        AlreadyStarted = 0x80,
        ModbusWriteCoilError = 0x100,
        ModbusReadRegisterError = 0x200,
    };
    Q_DECLARE_FLAGS(Errors, Error);

    // Db errors
    static const QString DB_ERROR;
    static const QString DB_GLOBALS;
    
    // Modbus errors
    static const QString OLD_DATA_ERROR;
    static const QString MODBUS_ERROR;

    // Autoklav errors
    static const QString DOOR_CLOSED_ERROR;
    static const QString BURNER_ERROR;
    static const QString WATER_SHORTAGE_ERROR;
    static const QString ALREADY_STARTED;
    static const QString MODBUS_WRITE_COIL_ERROR;
    static const QString MODBUS_READ_REGISTER_ERROR;

    static void setError(Error error);
    static void removeError(Error error);
    static Errors getErrors();
    static QVector<QString> getErrorsString();

private:
    inline static Errors errors;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(GlobalErrors::Errors)

#endif // GLOBALERRORS_H
