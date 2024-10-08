#include "logger.h"

Q_LOGGING_CATEGORY(logDebug,    "Debug")
Q_LOGGING_CATEGORY(logInfo,     "Info")
Q_LOGGING_CATEGORY(logWarning,  "Warning")
Q_LOGGING_CATEGORY(logCritical, "Critical")

void Logger::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (m_logFile == nullptr) {
        // Get appData folder
        auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

        // Create folder path if it doesn't exist
        QDir dir(path);
        if (!dir.exists())
            dir.mkpath(".");

        // Open log file
        m_logFile = std::make_unique<QFile>(path + "/log.txt");
        m_logFile->open(QFile::Append | QFile::Text);
    }

    QTextStream out(m_logFile.get());
    // Write the date of recording
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
    // By type determine to what level belongs message
    switch (type)
    {
    case QtInfoMsg:     out << "INF "; break;
    case QtDebugMsg:    out << "DBG "; break;
    case QtWarningMsg:  out << "WRN "; break;
    case QtCriticalMsg: out << "CRT "; break;
    case QtFatalMsg:    out << "FTL "; break;
    }
    // Write to the output category of the message and the message itself
    out << context.category << ": "
        << msg << "\n";
    out.flush();    // Clear the buffered data
}

void Logger::debug(QString message)
{
    qDebug(logDebug()) << message;
}

void Logger::info(QString message)
{
    qInfo(logInfo()) << message;
}

void Logger::warn(QString message)
{
    qWarning(logWarning()) << message;
}

void Logger::crit(QString message)
{
    qCritical(logCritical()) << message;
}

