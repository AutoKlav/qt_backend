#ifndef GLOBALS_H
#define GLOBALS_H

class Globals
{
public:
    struct Variables {
        double targetK;
        int serialDataTime;
        int stateMachineTick;
        double sterilizationTemp;
        double pasterizationTemp;
    };

    inline static double targetK = 0;
    inline static int serialDataTime = 0;
    inline static int stateMachineTick = 0;
    inline static double sterilizationTemp = 0;
    inline static double pasterizationTemp = 0;

    static bool setTargetK(double value);
    static bool setSerialDataTime(int value);
    static bool setStateMachineTick(int value);
    static bool setSterilizationTemp(double value);
    static bool setPasterizationTemp(double value);

    static Variables getVariables();
};

#endif // GLOBALS_H
