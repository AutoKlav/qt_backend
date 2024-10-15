#ifndef GLOBALERRORS_H
#define GLOBALERRORS_H

#include <QFlags>

class GlobalErrors
{
public:
    enum Error {
        DbError =           0x1,
        SerialError =       0x2,
        OldDataError =      0x4,
        SerialSendError =   0x8,
    };
    Q_DECLARE_FLAGS(Errors, Error);

    static const QString DB_ERROR;
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
