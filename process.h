#ifndef PROCESS_H
#define PROCESS_H

#include <QFile>
#include <QList>
#include <QObject>

struct ProcessType {
    int id;
    QString name;
    QString type;
    double customTemp;
    double finishTemp;
    double maintainTemp;
};

struct Bacteria {
    int id;
    QString name, description;
    double d0, z;
};

struct ProcessInfo {
    QString batchLTO, productName, productQuantity;
    QString processStart, processLength, targetHeatingTime, targetCoolingTime;;
    QString targetF;
    Bacteria bacteria;
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
    static QList<ProcessType> getProcessTypes();
    static QList<Bacteria> getBacteria();
    static int createProcessType(ProcessType processType);
    static int deleteProcessType(int id);

    int getId();
    ProcessInfo getInfo();
    bool setInfo(ProcessInfo newInfo);
    QString getName();

private:
    int saveProcess();

};

#endif // PROCESS_H
