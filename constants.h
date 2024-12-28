#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace CONSTANTS {

// Analog sensor constants
inline const QString TEMP = "temp"; // Temprature of autoklav
inline const QString TEMP_PIPE_EXPENSION = "tempPipe"; // Temprature of pipe expension
inline const QString TEMP_HEATERS = "tempHeaters"; // Temprature of heaters
inline const QString TEMP_WATER_TANK = "tempWaterTank"; // Temprature of water tank
inline const QString TEMP_K = "tempK"; // Tempreature of middle of can
inline const QString WATER_LEVEL = "waterLevel"; // ???
inline const QString PRESSURE = "pressure"; // Pressure of autoklav

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
