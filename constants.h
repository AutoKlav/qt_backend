#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace CONSTANTS {

// Analog sensor constants
inline const QString TEMP = "x";  // temp
inline const QString EXPANSION_TEMP = "y";  // expansionTemp
inline const QString HEATER_TEMP = "z";  // heaterTemp
inline const QString TANK_TEMP = "w";  // tankTemp
inline const QString TEMP_K = "v";  // tempK
inline const QString TANK_WATER_LEVEL = "u";  // tankWaterLevel
inline const QString PRESSURE = "t";  // pressure
inline const QString STEAM_PRESSURE = "s";  // steamPressure

// Digital input constants
inline const QString DOOR_CLOSED = "m";  // doorClosed
inline const QString BURNER_FAULT = "n";  // burnerFault
inline const QString WATER_SHORTAGE = "o";  // waterShortage

// Digital output constants
inline const QString FILL_TANK_WITH_WATER = "a";  // fillTankWithWater
inline const QString COOLING = "b";  // cooling
inline const QString TANK_HEATING = "c";  // tankHeating
inline const QString COOLING_HELPER = "d";  // coolingHelper
inline const QString AUTOKLAV_FILL = "e";  // autoklavFill
inline const QString WATER_DRAIN = "f";  // waterDrain

// default steam heating
inline QString HEATING = "g";  // heating
// electric heating
inline const QString ELECTRIC_HEATING = "i";  // electricHeating

inline const QString PUMP = "h";  // pump
inline const QString INCREASE_PRESSURE = "j";  // increasePressure
inline const QString EXTENSION_COOLING = "k";  // extensionCooling
inline const QString ALARM_SIGNAL = "l";  // alarmSignal

} // namespace CONSTANTS

#endif // CONSTANTS_H
