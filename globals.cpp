#include "globals.h"

#include "logger.h"
#include "dbmanager.h"

bool Globals::setVariable(const QString &key, const QString &newValue)
{
    auto it = variables.find(key);
    if (it == variables.end()) {
        Logger::warn(QString("setVariable: variable %1 doesn't exist").arg(key));
        return false;
    }

    VarRefType &varRef = it.value();

    // If the stored variant is a reference to int
    if (std::holds_alternative<std::reference_wrapper<int>>(varRef)) {
        bool ok = false;
        int value = newValue.toInt(&ok);
        if (ok) {
            std::get<std::reference_wrapper<int>>(varRef).get() = value;
        } else {
            Logger::warn(QString("Failed to parse %1 %2 to int").arg(key).arg(newValue));
            return false;
        }
    }
    // If the stored variant is a reference to double
    else if (std::holds_alternative<std::reference_wrapper<double>>(varRef)) {
        bool ok = false;
        double value = newValue.toDouble(&ok);
        if (ok) {
            std::get<std::reference_wrapper<double>>(varRef).get() = value;
        } else {
            Logger::warn(QString("Failed to parse %1 %2 to double").arg(key).arg(newValue));
            return false;
        }
    }

    return true;
}

bool Globals::updateVariable(const QString &key, const QString &newValue)
{
    if (setVariable(key, newValue)) {
        DbManager::instance().updateGlobal(key, newValue);
        return true;
    }

    return false;
}
