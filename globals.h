#ifndef GLOBALS_H
#define GLOBALS_H

#include <QHash>
#include <QString>
#include <QVariant>

class Globals
{
public:
    using VarRefType = std::variant<
        std::reference_wrapper<int>,
        std::reference_wrapper<double>
    >;

    inline static int stateMachineTick = 60000;
    inline static int dbTick = 120000;
    inline static int serialDataOldTime = 5000;
    inline static double k = 5;
    inline static double coolingThreshold = 50;
    inline static double expansionUpperTemp = 95;
    inline static double expansionLowerTemp = 90;
    inline static double heaterWaterLevel = 40;
    inline static double maintainWaterTankTemp = 95;
    inline static double tankWaterLevelThreshold = 95;

    inline static QHash<QString, VarRefType> variables = {
        {"stateMachineTick",        std::ref(stateMachineTick)},
        {"dbTick",                  std::ref(dbTick)},
        {"serialDataOldTime",       std::ref(serialDataOldTime)},
        {"k",                       std::ref(k)},
        {"coolingThreshold",        std::ref(coolingThreshold)},
        {"expansionUpperTemp",      std::ref(expansionUpperTemp)},
        {"expansionLowerTemp",      std::ref(expansionLowerTemp)},
        {"heaterWaterLevel",        std::ref(heaterWaterLevel)},
        {"maintainWaterTankTemp",   std::ref(maintainWaterTankTemp)},
        {"tankWaterLevelThreshold", std::ref(tankWaterLevelThreshold)}        
    };

    static bool setVariable(const QString &key, const QString &newValue);
    static bool updateVariable(const QString &key, const QString &newValue);
};

#endif // GLOBALS_H
