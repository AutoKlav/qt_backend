#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

/**
* @brief Constants related to the Controllino board, only ushort values are send and received through network.
*/
namespace CONSTANTS {
    // Analog sensor constants
    inline const ushort TEMP = 0;  // temp
    inline const ushort EXPANSION_TEMP = 1;  // expansionTemp
    inline const ushort HEATER_TEMP = 2;  // heaterTemp
    inline const ushort TANK_TEMP = 3;  // tankTemp
    inline const ushort TEMP_K = 4;  // tempK
    inline const ushort TANK_WATER_LEVEL = 5;  // tankWaterLevel
    inline const ushort PRESSURE = 6;  // pressure
    inline const ushort STEAM_PRESSURE = 7;  // steamPressure

    // Digital input constants
    inline const ushort DOOR_CLOSED = 8;  // doorClosed
    inline const ushort BURNER_FAULT = 9;  // burnerFault
    inline const ushort WATER_SHORTAGE = 10;  // waterShortage

    // Digital output constants
    inline const ushort FILL_TANK_WITH_WATER = 0;  // fillTankWithWater
    inline const ushort COOLING = 1;  // cooling
    inline const ushort TANK_HEATING = 2;  // tankHeating
    inline const ushort COOLING_HELPER = 3;  // coolingHelper
    inline const ushort AUTOKLAV_FILL = 4;  // autoklavFill
    inline const ushort WATER_DRAIN = 5;  // waterDrain

    // default steam heating
    inline ushort STEAM_HEATING = 6;  // heating
    // electric heating
    inline const ushort ELECTRIC_HEATING = 7;  // electricHeating

    inline const ushort PUMP = 8;  // pump
    inline const ushort INCREASE_PRESSURE = 9;  // increasePressure
    inline const ushort EXTENSION_COOLING = 10;  // extensionCooling
    inline const ushort ALARM_SIGNAL = 11;  // alarmSignal
}

#endif // CONSTANTS_H
