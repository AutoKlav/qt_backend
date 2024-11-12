#include "grpcserver.h"

#include <grpc++/grpc++.h>
#include "autoklav.grpc.pb.h"

#include <QtConcurrent/QtConcurrentRun>

#include "sensor.h"
#include "globals.h"
#include "processlog.h"
#include "statemachine.h"
#include "globalerrors.h"

using grpc::Status;

// Implement the Impl class
class GRpcServer::Impl
{
public:
    Impl();
    ~Impl();

    void shutdown();

private:
    // Define the service implementation here
    class AutoklavServiceImpl final : public autoklav::Autoklav::Service
    {
    public:
        Status getStatus(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Status *replay) override;
        Status getAllProcesses(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::ProcessInfoList *replay) override;
        Status getProcessLogs(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::ProcessLogList *replay) override;
        Status getVariables(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Variables *replay) override;
        Status setVariable(grpc::ServerContext *context, const autoklav::SetVariable *request, autoklav::Status *replay) override;
        Status startProcess(grpc::ServerContext *context, const autoklav::StartProcessRequest *request, autoklav::Status *replay) override;
        Status stopProcess(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Status *replay) override;
        Status getSensorValues(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::SensorValues *replay) override;
        Status getStateMachineValues(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::StateMachineValues *replay) override;
        // Custom helper function
        void setStatusReply(autoklav::Status *replay, int code);
    };

    AutoklavServiceImpl service;
    std::unique_ptr<grpc::Server> server;
};

GRpcServer::Impl::Impl()
{
    auto server_address("0.0.0.0:50061");

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    server = builder.BuildAndStart();

    // Start the server in a separate thread
    (void)QtConcurrent::run([this] {
        server->Wait();
    });
}

GRpcServer::Impl::~Impl()
{
    // Ensure the server is properly shutdown
    if (server) {
        server->Shutdown();
        server->Wait();
    }
}

void GRpcServer::Impl::shutdown()
{
    if (server) {
        server->Shutdown();
    }
}

GRpcServer::GRpcServer(QObject *parent)
    : QObject(parent), impl(std::make_unique<Impl>())
{
}

GRpcServer::~GRpcServer() = default;

void GRpcServer::shutdown()
{
    impl->shutdown();
}

// Implement the service methods

Status GRpcServer::Impl::AutoklavServiceImpl::getStatus(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Status *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    setStatusReply(replay, 0);
    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::getVariables(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Variables *replay)
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

Status GRpcServer::Impl::AutoklavServiceImpl::setVariable(grpc::ServerContext *context, const autoklav::SetVariable *request, autoklav::Status *replay)
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

/**
 * @brief Represents the status of a gRPC server operation.
 */
/**
 * @brief Represents the status of a gRPC operation.
 */
Status GRpcServer::Impl::AutoklavServiceImpl::startProcess(grpc::ServerContext *context, const autoklav::StartProcessRequest *request, autoklav::Status *replay)
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

Status GRpcServer::Impl::AutoklavServiceImpl::stopProcess(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Status *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    bool succ = StateMachine::instance().stop();

    setStatusReply(replay, !succ);
    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::getAllProcesses(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::ProcessInfoList *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    const auto processes = Process::getAllProcesses();

    for (const auto &process : processes) {

        auto processInfo = replay->add_processes();

        processInfo->set_id(process.id);
        processInfo->set_productname(process.productName.toStdString());
        processInfo->set_productquantity(process.productQuantity.toStdString());
        processInfo->set_bacteria(process.bacteria.toStdString());
        processInfo->set_description(process.description.toStdString());
        processInfo->set_processstart(process.processStart.toStdString());
        processInfo->set_processlength(process.processLength.toStdString());
    }

    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::getProcessLogs(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::ProcessLogList *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    const auto procesLogs = ProcessLog::getAllProcessLogsOrderedDesc(8);

    for (const auto &processLog : procesLogs) {
        auto processLogInfo = replay->add_processlogs();

        processLogInfo->set_id(processLog.processId);
        processLogInfo->set_time(processLog.timestamp.toInt());
        processLogInfo->set_temp(processLog.temp);
        processLogInfo->set_tempk(processLog.tempK);
        processLogInfo->set_pressure(processLog.pressure);
        processLogInfo->set_state(processLog.state.toInt());
        processLogInfo->set_dr(processLog.Dr);
        processLogInfo->set_fr(processLog.Fr);
        processLogInfo->set_r(processLog.r);
        processLogInfo->set_sumfr(processLog.sumFr);
        processLogInfo->set_sumr(processLog.sumr);        
    }

    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::getSensorValues(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::SensorValues *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);
    Q_UNUSED(replay);

    const auto sensorValues = Sensor::getValues();

    // Fetch error flags
    QVector<QString> errorStrings = GlobalErrors::getErrorsString();

    // If there is a serial error, send abort status
    for(const QString& errorString : errorStrings){
        if(errorString == GlobalErrors::SERIAL_ERROR){
            return Status(grpc::StatusCode::ABORTED, GlobalErrors::SERIAL_ERROR.toStdString());
        }        
    }
    
    replay->set_temp(sensorValues.temp);
    replay->set_tempk(sensorValues.tempK);
    replay->set_pressure(sensorValues.pressure);
    
    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::getStateMachineValues(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::StateMachineValues *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    const auto stateMachineValues = StateMachine::instance().calculateDrFrRValuesFromSensors(-1);

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

void GRpcServer::Impl::AutoklavServiceImpl::setStatusReply(autoklav::Status *replay, int code)
{
    replay->set_code(code);
    replay->set_errors(GlobalErrors::getErrors());
    replay->set_errorsstring(GlobalErrors::getErrorsString().join("|").toStdString());
}
