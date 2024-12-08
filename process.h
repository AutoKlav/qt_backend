#ifndef PROCESS_H
#define PROCESS_H

#include <QFile>
#include <QList>
#include <QObject>


struct ProcessInfo {
    QString productName, productQuantity;
    QString bacteria;
    QString description;
    QString processStart, processLength;
    QString targetF;
};

struct ProcessRow : ProcessInfo {
    int id;
};

class Process : public QObject
{
    Q_OBJECT

    int id = -1;
    QString name;
    ProcessInfo info;

public:
    explicit Process();
    explicit Process(QString name, ProcessInfo info, QObject *parent = nullptr);
    static QList<ProcessRow> getAllProcesses();
    static QList<QString> getFilteredProcessValues(QString columnName);
    static QMap<QString, QList<QString>> getFilteredTargetFAndProcessLengthValues(QString productName, QString productQuantity);

    int getId();
    ProcessInfo getInfo();
    bool setInfo(ProcessInfo newInfo);
    QString getName();

private:
    int saveProcess();

};

#endif // PROCESS_H
