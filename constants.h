#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace CONSTANTS {

// Analog sensor constants
inline const QString TEMP = "temp";
inline const QString TEMP_K = "tempK";
inline const QString PRESSURE = "pressure";
inline const QString STEAM_PRESSURE = "steamPressure";
inline const QString WATER_LEVEL = "waterLevel";

// Digital output constants
inline const QString FILL_TANK_WITH_WATER = "fillTankWithWater";
inline const QString COOLING = "cooling";
inline const QString TANK_HEATING = "tankHeating";
inline const QString COOLING_HELPER = "coolingHelper";
inline const QString AUTOKLAV_FILL = "autoklavFill";
inline const QString WATER_DRAIN = "waterDrain";
inline const QString HEATING = "heating";
inline const QString PUMP = "pump";
inline const QString ELECTRIC_HEATING = "electricHeating";
inline const QString INCREASE_PRESSURE = "increasePressure";
inline const QString EXTENSION_COOLING = "extensionCooling";
inline const QString ALARM_SIGNAL = "alarmSignal";

// Digital input constants
inline const QString DOOR_CLOSED = "doorClosed";
inline const QString BURNER_FAULT = "burnerFault";
inline const QString WATER_SHORTAGE = "waterShortage";

} // namespace CONSTANTS

#endif // CONSTANTS_H