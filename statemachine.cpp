#include "statemachine.h"

#include "logger.h"
#include "sensor.h"
#include "globals.h"

StateMachine::StateMachine(QObject *parent)
    : QObject{parent}
{
    state = State::READY;

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &StateMachine::tick);
}

double StateMachine::calcdTemp(double temp)
{
    switch (processConfig.type) {
    case Type::STERILIZATION:
        return Globals::sterilizationTemp - temp;
    case Type::PASTERIZATION:
        return Globals::pasterizationTemp - temp;
    case Type::CUSTOM:
        return processConfig.customTemp - temp;
    default:
        return Globals::sterilizationTemp - temp;
    }
}

bool StateMachine::start(ProcessConfig processConfig, ProcessInfo processInfo)
{
    if (isRunning())
        return false;

    this->processConfig = processConfig;
    state = State::STARTING;

    processStart = QDateTime::currentDateTime();
    processLog = new ProcessLog(processStart.toString(Qt::ISODate), processInfo, this);
    values = StateMachineValues();

    // timer->start(Globals::stateMachineTick);
    QMetaObject::invokeMethod(timer, "start", Qt::AutoConnection, Q_ARG(int, Globals::stateMachineTick));
    tick();

    return true;
}

bool StateMachine::stop()
{
    if (!isRunning())
        return false;

    Sensor::mapName["waterFill"]->send(0);
    Sensor::mapName["heating"]->send(0);
    Sensor::mapName["bypass"]->send(0);

    state = State::READY;
    values = StateMachineValues();

    return true;
}

inline bool StateMachine::isRunning()
{
    return state != State::READY;
}

StateMachineValues StateMachine::getValues()
{
    return values;
}

StateMachineValues StateMachine::calculateDrFrRValuesFromSensors()
{
    StateMachineValues smValues;

    auto sensorValues = Sensor::getValues();

    smValues.temp = sensorValues.temp;
    smValues.tempK = sensorValues.tempK;
    smValues.pressure = sensorValues.pressure;

    smValues.dTemp = calcdTemp(smValues.tempK);

    smValues.Dr = qPow(10,  0.1 * smValues.dTemp) * (Globals::stateMachineTick / 60000.0);
    smValues.Fr = qPow(10, -0.1 * smValues.dTemp) * (Globals::stateMachineTick / 60000.0);
    smValues.r = 5 * smValues.Fr;

    if (isRunning()) {
        smValues.time = processStart.msecsTo(QDateTime::currentDateTime());

        smValues.sumFr = values.sumFr + smValues.Fr;
        smValues.sumr = values.sumr + smValues.r;
    } else {
        smValues.time = 0;
        smValues.sumFr = 0;
        smValues.sumr = 0;
    }

    return smValues;
}

void StateMachine::tick()
{
    values = calculateDrFrRValuesFromSensors();

    if (processLog && isRunning())
        processLog->appendLog(values);

    switch (state) {
    case State::READY:
        break;

    case State::STARTING:
        Logger::info("StateMachine: Starting");

        Sensor::mapName["waterFill"]->send(1);  // Ukljucivanje punjenja vodom
        Sensor::mapName["heating"]->send(1);    // Ukljucivanje grijaca
        Sensor::mapName["bypass"]->send(1);     // Ukljucivanje bypassa

        state = State::FILLING;
        Logger::info("StateMachine: Filling");
        break;

    case State::FILLING:
        // Ceka dok se tlak ne popne na 0.5 bara
        if (values.pressure < 0.5)
            break;

        Sensor::mapName["waterFill"]->send(0);  // Iskljucivanje punjenje vodom
        Sensor::mapName["bypass"]->send(0);     // Iskljucivanje bypassa
        Sensor::mapName["pump"]->send(1);       // Ukljucivanje cirkulacione pumpe
        Sensor::mapName["inPressure"]->send(1);   // Ukljucivanje tlaka 2bara

        state = State::HEATING;
        Logger::info("StateMachine: Heating");
        break;

    case State::HEATING:
        // Odrzava zadanu temperaturu u +-1
        if (values.temp > processConfig.maintainTemp + 1)
            Sensor::mapName["heating"]->send(0);    // Iskljucivanje grijaca
        else if (values.temp < processConfig.maintainTemp - 1)
            Sensor::mapName["heating"]->send(1);    // Ukljucivanje grijaca

        // Odrzava zadani tlak u +-0.05
        if (values.pressure > processConfig.maintainPressure + 0.05)
            Sensor::mapName["inPressure"]->send(0);    // Iskljucivanje tlaka 2bara
        else if (values.pressure < processConfig.maintainPressure - 0.05)
            Sensor::mapName["inPressure"]->send(1);    // Ukljucivanje tlaka 2bara

        // Grijanje dok ne dode do zadanone F vrijednosti ili prolaska vremena
        if (processConfig.mode == Mode::TARGETF) {
            if (values.sumFr < processConfig.targetF)
                break;
        } else if (processConfig.mode == Mode::TIME) {
            if (values.time < processConfig.targetTime)
                break;
        }

        Sensor::mapName["heating"]->send(0);        // Iskljucivanje grijaca
        Sensor::mapName["inPressure"]->send(0);       // Iskljuciti tlak 2bara
        Sensor::mapName["cooling"]->send(1);        // Otvoriti magnetski ventil hladjenja
        Sensor::mapName["bypass"]->send(1);         // Ukljucivanje bypassa

        state = State::COOLING;
        Logger::info("StateMachine: Cooling");
        break;

    case State::COOLING:
        // Hladnjenje do zadane temperature
        if (values.tempK > processConfig.finishTemp)
            break;

        Sensor::mapName["cooling"]->send(0);        // Iskljuciti hladjenje u autoklavu
        Sensor::mapName["bypass"]->send(0);         // Iskljucivanje bypassa

        state = State::FINISHING;
        Logger::info("StateMachine: Finishing");
        break;

    case State::FINISHING:
        Sensor::mapName["pump"]->send(0);           // Iskljucivanje cirkulacione pumpe

        state = State::FINISHED;
        Logger::info("StateMachine: Finished");
        break;

    case State::FINISHED:
        timer->stop();

        Logger::info("StateMachine: Ready");

        if (processLog) {
            auto processLogInfo = processLog->getInfo();
            processLogInfo.processLength = QString::number(values.time);
            processLog->setInfo(processLogInfo);
        }

        state = State::READY;
        values = StateMachineValues();

        break;
    }
}

StateMachine &StateMachine::instance()
{
    static StateMachine _instance;
    return _instance;
}
