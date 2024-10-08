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

    inline static double targetK = 0.0;
    inline static int serialDataTime = 3000;
    inline static int stateMachineTick = 60000;
    inline static double sterilizationTemp = 121.1;
    inline static double pasterizationTemp = 70.0;

    static bool setTargetK(double value);
    static bool setSerialDataTime(int value);
    static bool setStateMachineTick(int value);
    static bool setSterilizationTemp(double value);
    static bool setPasterizationTemp(double value);

    static Variables getVariables();
};

#endif // GLOBALS_H
