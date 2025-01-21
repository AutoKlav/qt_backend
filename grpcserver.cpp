#include "grpcserver.h"

#include <grpc++/grpc++.h>
#include "autoklav.grpc.pb.h"

#include <QCoreApplication>
#include <QtConcurrent/QtConcurrentRun>

#include "sensor.h"
#include "globals.h"
#include "processlog.h"
#include "statemachine.h"
#include "globalerrors.h"
#include "constants.h"
#include "invokeonmainthread.h"
#include "logger.h"

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
        Status getDistinctProcessValues(grpc::ServerContext *context, const autoklav::ProcessFilterRequest *request, autoklav::FilteredProcessList *replay) override;
        Status getFilteredModeValues(grpc::ServerContext *context, const autoklav::ProcessModeFilterRequest *request, autoklav::FilteredModeProcessList *replay) override;
        Status getAllProcessTypes(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::ProcessTypesList *replay) override;
        Status getProcessLogs(grpc::ServerContext *context, const autoklav::ProcessLogRequest *request, autoklav::ProcessLogList *replay) override;
        Status getVariables(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Variables *replay) override;
        Status setVariable(grpc::ServerContext *context, const autoklav::SetVariable *request, autoklav::Status *replay) override;
        Status getBacteria(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::BacteriaList *replay) override;
        Status createProcessType(grpc::ServerContext *context, const autoklav::ProcessTypeRequest *request, autoklav::Status *replay) override;
        Status deleteProcessType(grpc::ServerContext *context, const autoklav::TypeRequest *request, autoklav::Status *replay) override;
        Status startProcess(grpc::ServerContext *context, const autoklav::StartProcessRequest *request, autoklav::Status *replay) override;
        Status stopProcess(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Status *replay) override;
        Status getSensorPinValues(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::SensorValues *replay) override;
        Status getSensorRelayValues(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::SensorRelayValues *replay) override;
        Status updateSensor(grpc::ServerContext *context, const autoklav::UpdateSensorRequest *request, autoklav::Status *replay) override;
        Status getStateMachineValues(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::StateMachineValues *replay) override;
        Status setRelayStatus(grpc::ServerContext *context, const autoklav::SetVariable *request, autoklav::Status *replay) override;
        Status setStateMachineState(grpc::ServerContext *context, const autoklav::SetState *request, autoklav::Status *replay) override;

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

    Logger::info("GRpc server started");

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

    replay->set_statemachinetick(Globals::stateMachineTick);
    replay->set_dbtick(Globals::dbTick);
    replay->set_k(Globals::k);
    replay->set_coolingthreshold(Globals::coolingThreshold);
    replay->set_expansionuppertemp(Globals::expansionUpperTemp);
    replay->set_expansionlowertemp(Globals::expansionLowerTemp);
    replay->set_heaterwaterlevel(Globals::heaterWaterLevel);
    replay->set_maintainwatertanktemp(Globals::maintainWaterTankTemp);

    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::setVariable(grpc::ServerContext *context, const autoklav::SetVariable *request, autoklav::Status *replay)
{
    Q_UNUSED(context);

    const auto name = QString::fromUtf8(request->name().c_str());
    const auto value = QString::fromUtf8(request->value().c_str());

    bool success = invokeOnMainThreadBlocking([name, value](){
        return Globals::updateVariable(name, value);
    });

    setStatusReply(replay, !success);
    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::setRelayStatus(grpc::ServerContext *context, const autoklav::SetVariable *request, autoklav::Status *replay)
{
    Q_UNUSED(context);

    const auto name = QString::fromUtf8(request->name().c_str());
    const auto value = QString::fromUtf8(request->value()).toUInt();

    bool success = invokeOnMainThreadBlocking([name, value](){
        return Sensor::setRelayState(name, value);
    });

    setStatusReply(replay, !success);
    return Status::OK;
}


Status GRpcServer::Impl::AutoklavServiceImpl::setStateMachineState(grpc::ServerContext *context, const autoklav::SetState *request, autoklav::Status *replay)
{
    Q_UNUSED(context);

    const auto state = request->state();

    bool success = invokeOnMainThreadBlocking([state](){
        return  StateMachine::instance().setState(state);
    });

    setStatusReply(replay, !success);
    return Status::OK;
}


Status GRpcServer::Impl::AutoklavServiceImpl::updateSensor(grpc::ServerContext *context, const autoklav::UpdateSensorRequest *request, autoklav::Status *replay)
{
    Q_UNUSED(context);

    const auto name = QString::fromUtf8(request->name().c_str());
    const auto minValue = request->minvalue();
    const auto maxValue = request->maxvalue();

    bool success = invokeOnMainThreadBlocking([name, minValue, maxValue](){
        return Sensor::updateSensor(name, minValue, maxValue);
    });

    setStatusReply(replay, !success);
    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::startProcess(grpc::ServerContext *context, const autoklav::StartProcessRequest *request, autoklav::Status *replay)
{
    Q_UNUSED(context);

    const StateMachine::ProcessConfig processConfig = {
        .type = static_cast<StateMachine::Type>(request->processconfig().type()),
        .heatingType = static_cast<StateMachine::HeatingType>(request->processconfig().heatingtype()),
        .customTemp = request->processconfig().customtemp(),
        .mode = static_cast<StateMachine::Mode>(request->processconfig().mode()),        
        .maintainTemp = request->processconfig().maintaintemp(),
        .finishTemp = request->processconfig().finishtemp(),
    };

    if(processConfig.heatingType == StateMachine::ELECTRIC)
        CONSTANTS::HEATING = CONSTANTS::ELECTRIC_HEATING;

    const Bacteria bacteria = {
        .id = static_cast<int>(request->processinfo().bacteria().id()),
        .name = QString::fromUtf8(request->processinfo().bacteria().name()).trimmed(),
        .description = QString::fromUtf8(request->processinfo().bacteria().description()).trimmed(),
        .d0 = request->processinfo().bacteria().d0(),
        .z = request->processinfo().bacteria().z()
    };

    const ProcessInfo processInfo = {
        .batchLTO = QString::fromUtf8(request->processinfo().batchlto()).trimmed(),
        .productName = QString::fromUtf8(request->processinfo().productname()).trimmed(),
        .productQuantity = QString::fromUtf8(request->processinfo().productquantity()).trimmed(),
        .processStart = QString::fromUtf8(request->processinfo().processstart()),
        .processLength = QString::fromUtf8(request->processinfo().processlength()),
        .targetHeatingTime = QString::fromUtf8(request->processinfo().targetheatingtime()),
        .targetCoolingTime = QString::fromUtf8(request->processinfo().targetcoolingtime()),
        .targetF = QString::fromUtf8(request->processinfo().targetf()),        
        .bacteria = bacteria,        
    };

    bool success = invokeOnMainThreadBlocking([processConfig, processInfo](){
        return StateMachine::instance().start(processConfig, processInfo);
    });

    setStatusReply(replay, !success);
    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::createProcessType(grpc::ServerContext *context, const autoklav::ProcessTypeRequest *request, autoklav::Status *replay)
{
    Q_UNUSED(context);

    const ProcessType processType = {
        .name = QString::fromStdString(request->name()),
        .type = QString::fromStdString(request->type()),
        .customTemp = request->customtemp(),
        .finishTemp = request->finishtemp(),
        .maintainTemp = request->maintaintemp()
    };

    bool success = invokeOnMainThreadBlocking([processType](){
        return Process::createProcessType(processType);
    });

    setStatusReply(replay, !success);
    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::deleteProcessType(grpc::ServerContext *context, const autoklav::TypeRequest *request, autoklav::Status *replay)
{
    Q_UNUSED(context);

    const auto id = request->id();

    bool success = invokeOnMainThreadBlocking([id](){
        return Process::deleteProcessType(id);
    });

    setStatusReply(replay, !success);
    return Status::OK;
}


Status GRpcServer::Impl::AutoklavServiceImpl::stopProcess(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Status *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    bool success = invokeOnMainThreadBlocking([](){
        return StateMachine::instance().stop();
    });

    setStatusReply(replay, !success);
    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::getAllProcesses(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::ProcessInfoList *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    const auto processes = Process::getAllProcesses();

    for (const auto& process : processes) {
        auto processInfo = replay->add_processes();

        processInfo->set_id(process.id);
        processInfo->set_batchlto(process.batchLTO.toStdString());

        // Access and populate the nested Bacteria message
        auto bacteriaMessage = processInfo->mutable_bacteria();
        bacteriaMessage->set_id(process.bacteria.id);
        bacteriaMessage->set_name(process.bacteria.name.toStdString());
        bacteriaMessage->set_description(process.bacteria.description.toStdString());
        bacteriaMessage->set_d0(process.bacteria.d0);
        bacteriaMessage->set_z(process.bacteria.z);

        processInfo->set_productname(process.productName.toStdString());
        processInfo->set_productquantity(process.productQuantity.toStdString());
        processInfo->set_processstart(process.processStart.toStdString());
        processInfo->set_targetf(process.targetF.toStdString());
        processInfo->set_processlength(process.processLength.toStdString());
    }

    return Status::OK;
}


Status GRpcServer::Impl::AutoklavServiceImpl::getBacteria(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::BacteriaList *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    const auto bacteria = invokeOnMainThreadBlocking([](){
        return Process::getBacteria();
    });

    for (const auto &bacterium : bacteria) {

        auto bacteria = replay->add_bacteria();

        bacteria->set_id(bacterium.id);
        bacteria->set_name(bacterium.name.toStdString());
        bacteria->set_description(bacterium.description.toStdString());
        bacteria->set_d0(bacterium.d0);
        bacteria->set_z(bacterium.z);
    }

    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::getDistinctProcessValues(grpc::ServerContext *context, const autoklav::ProcessFilterRequest *request, autoklav::FilteredProcessList *replay)
{
    Q_UNUSED(context);

    auto columnName = request->columnname();

    const auto processes = invokeOnMainThreadBlocking([columnName](){
        return Process::getFilteredProcessValues(QString::fromStdString(columnName));
    });

    for (const auto &process : processes) {
        replay->add_values(process.toStdString());
    }

    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::getFilteredModeValues(grpc::ServerContext *context, const autoklav::ProcessModeFilterRequest *request, autoklav::FilteredModeProcessList *replay)
{
    Q_UNUSED(context);

    const auto productName = QString::fromStdString(request->productname());
    const auto productQuantity = QString::fromStdString(request->productquantity());

    // Call to the function that retrieves the values from the database
    const auto filteredMap = invokeOnMainThreadBlocking([productName, productQuantity](){
        return Process::getFilteredTargetFAndProcessLengthValues(productName, productQuantity);
    });

    // Extract the "targetF" and "processLength" lists from the filteredMap
    const auto targetFList = filteredMap.value("targetF");
    const auto processLengthList = filteredMap.value("processLength");

    // Add all targetF values to the replay response
    for (const auto &targetF : targetFList) {
        replay->add_targetfvalues(targetF.toStdString());
    }

    // Add all processLength values to the replay response
    for (const auto &processLength : processLengthList) {
        replay->add_processlengthvalues(processLength.toStdString());
    }

    return grpc::Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::getAllProcessTypes(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::ProcessTypesList *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    const auto processTypes = invokeOnMainThreadBlocking([](){
        return Process::getProcessTypes();
    });

    for (const auto &processType : processTypes) {
        auto processTypeInfo = replay->add_processtypes();

        processTypeInfo->set_id(processType.id);
        processTypeInfo->set_name(processType.name.toStdString());
        processTypeInfo->set_type(processType.type.toStdString());
        processTypeInfo->set_customtemp(processType.customTemp);
        processTypeInfo->set_finishtemp(processType.finishTemp);
        processTypeInfo->set_maintaintemp(processType.maintainTemp);
    }

    return Status::OK;

}

Status GRpcServer::Impl::AutoklavServiceImpl::getProcessLogs(grpc::ServerContext *context, const autoklav::ProcessLogRequest *request, autoklav::ProcessLogList *replay)
{
    Q_UNUSED(context);

    std::vector<ProcessLogInfoRow> allProcessLogs;

    for (const auto &id : request->ids()) {
        const auto processLogs = invokeOnMainThreadBlocking([id](){
            return ProcessLog::getAllProcessLogs(id);
        });

        allProcessLogs.insert(allProcessLogs.end(), processLogs.begin(), processLogs.end());
    }

    for (const auto &processLog : allProcessLogs) {
        auto processLogInfo = replay->add_processlogs();

        processLogInfo->set_id(processLog.processId);
        processLogInfo->set_timestamp(processLog.timestamp.toStdString());
        
        auto sensorValues = processLogInfo->mutable_sensorvalues();
        sensorValues->set_temp(processLog.temp);
        sensorValues->set_expansiontemp(processLog.expansionTemp);
        sensorValues->set_heatertemp(processLog.heaterTemp);
        sensorValues->set_tanktemp(processLog.tankTemp);
        sensorValues->set_tempk(processLog.tempK);
        sensorValues->set_tankwaterlevel(processLog.tankWaterLevel);
        sensorValues->set_pressure(processLog.pressure);
        sensorValues->set_steampressure(processLog.steamPressure);
        sensorValues->set_doorclosed(processLog.doorClosed);
        sensorValues->set_burnerfault(processLog.burnerFault);
        sensorValues->set_watershortage(processLog.waterShortage);                

        processLogInfo->set_state(processLog.state);
        processLogInfo->set_dr(processLog.Dr);
        processLogInfo->set_fr(processLog.Fr);
        processLogInfo->set_r(processLog.r);
        processLogInfo->set_sumfr(processLog.sumFr);
        processLogInfo->set_sumr(processLog.sumr);
    }

    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::getSensorPinValues(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::SensorValues *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    const auto sensorValues = invokeOnMainThreadBlocking([](){
        return Sensor::getPinValues();
    });

    // Fetch error flags
    QVector<QString> errorStrings = GlobalErrors::getErrorsString();

    // If there is a serial error, send abort status
    for(const QString& errorString : errorStrings){
        if(errorString == GlobalErrors::SERIAL_ERROR){
            return Status(grpc::StatusCode::ABORTED, GlobalErrors::SERIAL_ERROR.toStdString());
        }
    }

    replay->set_temp(sensorValues.temp);
    replay->set_expansiontemp(sensorValues.expansionTemp);
    replay->set_heatertemp(sensorValues.heaterTemp);
    replay->set_tanktemp(sensorValues.tankTemp);
    replay->set_tempk(sensorValues.tempK);
    replay->set_tankwaterlevel(sensorValues.tankWaterLevel);
    replay->set_pressure(sensorValues.pressure);
    replay->set_steampressure(sensorValues.steamPressure);

    replay->set_doorclosed(sensorValues.doorClosed);
    replay->set_burnerfault(sensorValues.burnerFault);
    replay->set_watershortage(sensorValues.waterShortage);

    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::getSensorRelayValues(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::SensorRelayValues *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);
    Q_UNUSED(replay);

    const auto sensorRelayValues = invokeOnMainThreadBlocking([](){
        return Sensor::getRelayValues();
    });

    // Fetch error flags
    QVector<QString> errorStrings = GlobalErrors::getErrorsString();

    // If there is a serial error, send abort status
    for(const QString& errorString : errorStrings){
        if(errorString == GlobalErrors::SERIAL_ERROR){
            return Status(grpc::StatusCode::ABORTED, GlobalErrors::SERIAL_ERROR.toStdString());
        }
    }
    
    replay->set_filltankwithwater(sensorRelayValues.fillTankWithWater);
    replay->set_cooling(sensorRelayValues.cooling);
    replay->set_tankheating(sensorRelayValues.tankHeating);
    replay->set_coolinghelper(sensorRelayValues.coolingHelper);
    replay->set_autoklavfill(sensorRelayValues.autoklavFill);
    replay->set_waterdrain(sensorRelayValues.waterDrain);
    replay->set_heating(sensorRelayValues.heating);
    replay->set_pump(sensorRelayValues.pump);
    replay->set_electricheating(sensorRelayValues.electricHeating);
    replay->set_increasepressure(sensorRelayValues.increasePressure);
    replay->set_extensioncooling(sensorRelayValues.extensionCooling);
    replay->set_alarmsignal(sensorRelayValues.alarmSignal);
    
    return Status::OK;
}

Status GRpcServer::Impl::AutoklavServiceImpl::getStateMachineValues(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::StateMachineValues *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    const auto stateMachineValues = invokeOnMainThreadBlocking([](){
        return StateMachine::instance().calculateStateMachineValues();
    });
     
    replay->set_elapsedtime(stateMachineValues.time);

    auto sensorValues = replay->mutable_sensorvalues();
    sensorValues->set_temp(stateMachineValues.temp);
    sensorValues->set_expansiontemp(stateMachineValues.expansionTemp);
    sensorValues->set_heatertemp(stateMachineValues.heaterTemp);
    sensorValues->set_tanktemp(stateMachineValues.tankTemp);
    sensorValues->set_tempk(stateMachineValues.tempK);
    sensorValues->set_tankwaterlevel(stateMachineValues.tankWaterLevel);
    sensorValues->set_pressure(stateMachineValues.pressure);
    sensorValues->set_steampressure(stateMachineValues.steamPressure);

    sensorValues->set_doorclosed(stateMachineValues.doorClosed);
    sensorValues->set_burnerfault(stateMachineValues.burnerFault);
    sensorValues->set_watershortage(stateMachineValues.waterShortage);

    replay->set_dtemp(stateMachineValues.dTemp);
    replay->set_state(stateMachineValues.state);
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
