#ifndef GLOBALS_H
#define GLOBALS_H

class Globals
{
public:
    struct Variables {
        int serialDataTime;
        int stateMachineTick;        
    };

    inline static int serialDataTime = 0;
    inline static int stateMachineTick = 0;    

    static bool setSerialDataTime(int value);
    static bool setStateMachineTick(int value);
    
    static Variables getVariables();
};

#endif // GLOBALS_H
