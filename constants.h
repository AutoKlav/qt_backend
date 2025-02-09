#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

/**
* @brief Constants related to the Controllino board, only ushort values are send and received through network.
*/
namespace CONSTANTS {
    // Analog sensor constants
    inline const ushort TEMP = 1;  // temp
    inline const ushort EXPANSION_TEMP = 2;  // expansionTemp
    inline const ushort HEATER_TEMP = 3;  // heaterTemp
    inline const ushort TANK_TEMP = 4;  // tankTemp
    inline const ushort TEMP_K = 5;  // tempK
    inline const ushort TANK_WATER_LEVEL = 6;  // tankWaterLevel
    inline const ushort PRESSURE = 7;  // pressure
    inline const ushort STEAM_PRESSURE = 8;  // steamPressure

    // Digital input constants
    inline const ushort DOOR_CLOSED = 9;  // doorClosed
    inline const ushort BURNER_FAULT = 10;  // burnerFault
    inline const ushort WATER_SHORTAGE = 11;  // waterShortage

    // Digital output constants
    inline const ushort FILL_TANK_WITH_WATER = 1;  // fillTankWithWater
    inline const ushort COOLING = 2;  // cooling
    inline const ushort TANK_HEATING = 3;  // tankHeating
    inline const ushort COOLING_HELPER = 4;  // coolingHelper
    inline const ushort AUTOKLAV_FILL = 5;  // autoklavFill
    inline const ushort WATER_DRAIN = 6;  // waterDrain

    // default steam heating
    inline ushort STEAM_HEATING = 7;  // heating
    // electric heating
    inline const ushort ELECTRIC_HEATING = 8;  // electricHeating

    inline const ushort PUMP = 9;  // pump
    inline const ushort INCREASE_PRESSURE = 10;  // increasePressure
    inline const ushort EXTENSION_COOLING = 11;  // extensionCooling
    inline const ushort ALARM_SIGNAL = 12;  // alarmSignal
}

#endif // CONSTANTS_H
