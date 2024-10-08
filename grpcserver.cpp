#include "grpcserver.h"

#include <QtConcurrent>
#include <QSqlDatabase>

#include "sensor.h"
#include "globals.h"
#include "processlog.h"
#include "autoklav.pb.h"
#include "globalerrors.h"
#include "statemachine.h"

using grpc::ServerBuilder;

namespace {
    std::shared_ptr<Server> buildAndStartService(AutoklavServiceImplementation & service_)
    {
      auto server_address("0.0.0.0:50061");

      return ServerBuilder()
          .AddListeningPort(server_address, grpc::InsecureServerCredentials())
          .RegisterService(&service_)
          .BuildAndStart();
    }
}

GRpcServer::GRpcServer(QObject *parent)
    : QObject(parent), server(buildAndStartService(this->service))
{
    (void)QtConcurrent::run([=] {
        this->server->Wait();
    });
}

void GRpcServer::shutdown()
{
    server->Shutdown();
}

Status AutoklavServiceImplementation::getStatus(grpc::ServerContext *context, const Autoklav::Empty *request, Autoklav::Status *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    setStatusReply(replay, 0);
    return Status::OK;
}

Status AutoklavServiceImplementation::getVariables(grpc::ServerContext *context, const Autoklav::Empty *request, Autoklav::Variables *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    auto variables = Globals::getVariables();

    replay->set_targetk(variables.targetK);
    replay->set_serialdatatime(variables.serialDataTime);
    replay->set_statemachinetick(variables.stateMachineTick);
    replay->set_sterilizationtemp(variables.sterilizationTemp);
    replay->set_pasterizationtemp(variables.pasterizationTemp);

    return Status::OK;
}

Status AutoklavServiceImplementation::setVariable(grpc::ServerContext *context, const Autoklav::SetVariable *request, Autoklav::Status *replay)
{
    Q_UNUSED(context);

    const auto name = QString::fromUtf8(request->name().c_str());
    const auto value = QString::fromUtf8(request->value().c_str());

    bool succ = false;
    if (name == "targetK") {
        succ = Globals::setTargetK(value.toDouble());
    } else if (name == "serialDataTime") {
        succ = Globals::setSerialDataTime(value.toInt());
    } else if (name == "stateMachineTick") {
        succ = Globals::setStateMachineTick(value.toInt());
    } else if (name == "sterilizationTemp") {
        succ = Globals::setSterilizationTemp(value.toDouble());
    } else if (name == "pasterizationTemp") {
        succ = Globals::setPasterizationTemp(value.toDouble());
    }

    setStatusReply(replay, !succ);
    return Status::OK;
}

Status AutoklavServiceImplementation::startProcess(grpc::ServerContext *context, const Autoklav::StartProcessRequest *request, Autoklav::Status *replay)
{
    Q_UNUSED(context);

    const StateMachine::ProcessConfig processConfig = {
        .type = static_cast<StateMachine::Type>(request->processconfig().type()),
        .customTemp = request->processconfig().customtemp(),
        .mode = static_cast<StateMachine::Mode>(request->processconfig().mode()),
        .targetF = request->processconfig().targetf(),
        .targetTime = request->processconfig().targettime(),
        .maintainTemp = request->processconfig().maintaintemp(),
        .maintainPressure = request->processconfig().maintainpressure(),
        .finishTemp = request->processconfig().finishtemp(),
    };

    const ProcessInfo processInfo = {
        .productName = QString::fromUtf8(request->processinfo().productname()),
        .productQuantity = QString::fromUtf8(request->processinfo().productquantity()),
        .bacteria = QString::fromUtf8(request->processinfo().bacteria()),
        .description = QString::fromUtf8(request->processinfo().description()),
        .processStart = QString::fromUtf8(request->processinfo().processstart()),
        .processLength = QString::fromUtf8(request->processinfo().processlength()),
    };

    bool succ = StateMachine::instance().start(processConfig, processInfo);

    setStatusReply(replay, !succ);
    return Status::OK;
}

Status AutoklavServiceImplementation::stopProcess(grpc::ServerContext *context, const Autoklav::Empty *request, Autoklav::Status *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    bool succ = StateMachine::instance().stop();

    setStatusReply(replay, !succ);
    return Status::OK;
}

Status AutoklavServiceImplementation::getSensorValues(grpc::ServerContext *context, const Autoklav::Empty *request, Autoklav::SensorValues *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);
    Q_UNUSED(replay);

    const auto sensorValues = Sensor::getValues();

    replay->set_temp(sensorValues.temp);
    replay->set_tempk(sensorValues.tempK);
    replay->set_pressure(sensorValues.pressure);

    return Status::OK;
}

Status AutoklavServiceImplementation::getStateMachineValues(grpc::ServerContext *context, const Autoklav::Empty *request, Autoklav::StateMachineValues *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    const auto stateMachineValues = StateMachine::instance().calcValues();

    replay->set_time(stateMachineValues.time);
    replay->set_temp(stateMachineValues.temp);
    replay->set_tempk(stateMachineValues.tempK);
    replay->set_dtemp(stateMachineValues.dTemp);
    replay->set_pressure(stateMachineValues.pressure);
    replay->set_dr(stateMachineValues.Dr);
    replay->set_fr(stateMachineValues.Fr);
    replay->set_r(stateMachineValues.r);
    replay->set_sumfr(stateMachineValues.sumFr);
    replay->set_sumr(stateMachineValues.sumr);

    return Status::OK;
}

void AutoklavServiceImplementation::setStatusReply(Autoklav::Status *replay, int code)
{
    replay->set_code(code);
    replay->set_errors(GlobalErrors::getErrors());
    replay->set_errorsstring(GlobalErrors::getErrorsString().join("|").toStdString());
}
