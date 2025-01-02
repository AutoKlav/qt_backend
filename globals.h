#ifndef GLOBALS_H
#define GLOBALS_H

class Globals
{
public:
    struct Variables {
        int serialDataTime;
        int stateMachineTick;
        double k;
    };

    inline static int serialDataTime = 0;
    inline static int stateMachineTick = 0;
    inline static double k = 1;
    inline static double coolingThreshold = 50;

    static bool setSerialDataTime(int value);
    static bool setStateMachineTick(int value);
    static bool setK(double value);
    
    static Variables getVariables();
};

#endif // GLOBALS_H
