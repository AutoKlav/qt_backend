#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

/**
* @brief Constants related to the Controllino board, only ushort values are send and received through network.
*/
namespace CONSTANTS {
    // Analog sensor constants
    inline const ushort TEMP = 2;  // temp
    inline const ushort TEMP_K = 3;  // tempK
    inline const ushort EXPANSION_TEMP = 4;  // expansionTemp
    inline const ushort HEATER_TEMP = 5;  // heaterTemp
    inline const ushort TANK_TEMP = 6;  // tankTemp
    inline const ushort TANK_WATER_LEVEL = 7;  // tankWaterLevel
    inline const ushort STEAM_PRESSURE = 8;  // steamPressure
    inline const ushort PRESSURE = 9;  // pressure

    // Digital input constants
    inline const ushort DOOR_CLOSED = 10;  // doorClosed
    inline const ushort BURNER_FAULT = 11;  // burnerFault
    inline const ushort WATER_SHORTAGE = 12;  // waterShortage

    // Digital output constants
    inline const ushort FILL_TANK_WITH_WATER = 0;  // fillTankWithWater
    inline const ushort COOLING = 1;  // cooling
    inline const ushort TANK_HEATING = 2;  // tankHeating
    inline const ushort COOLING_HELPER = 3;  // coolingHelper
    inline const ushort AUTOKLAV_FILL = 4;  // autoklavFill
    inline const ushort WATER_DRAIN = 5;  // waterDrain

    // default steam heating
    inline ushort STEAM_HEATING = 6;  // heating

    inline const ushort PUMP = 7;  // pump

    // electric heating
    inline const ushort ELECTRIC_HEATING = 11;  // electricHeating

    inline const ushort INCREASE_PRESSURE = 8;  // increasePressure
    inline const ushort EXTENSION_COOLING = 10;  // extensionCooling
    inline const ushort ALARM_SIGNAL = 9;  // alarmSignal
}

#endif // CONSTANTS_H
