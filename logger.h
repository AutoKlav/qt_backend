#ifndef LOGGER_H
#define LOGGER_H

#include <QLoggingCategory>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>

Q_DECLARE_LOGGING_CATEGORY(logDebug)
Q_DECLARE_LOGGING_CATEGORY(logInfo)
Q_DECLARE_LOGGING_CATEGORY(logWarning)
Q_DECLARE_LOGGING_CATEGORY(logCritical)

class Logger
{
public:
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    static void debug(QString message);
    static void info(QString message);
    static void warn(QString message);
    static void crit(QString message);

private:
    inline static std::unique_ptr<QFile> m_logFile = nullptr;

signals:

};

#endif // LOGGER_H
