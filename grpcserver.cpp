// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "grpcserver.h"

#include "autoklav.grpc.pb.h"

#include "sensor.h"
#include "globals.h"
#include "processlog.h"
#include "statemachine.h"
#include "globalerrors.h"

#include <QThread>
#include <QDebug>
#include <QRandomGenerator>

#include <grpc++/grpc++.h>
#include <memory>

namespace {

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

// Logic and data behind the server's behavior.
class AutoklavServiceImpl final : public autoklav::Autoklav::Service
{
    Status getStatus(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Status *replay) override;
    Status getVariables(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Variables *replay) override;
    Status setVariable(grpc::ServerContext *context, const autoklav::SetVariable *request, autoklav::Status *replay) override;
    Status startProcess(grpc::ServerContext *context, const autoklav::StartProcessRequest *request, autoklav::Status *replay) override;
    Status stopProcess(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Status *replay) override;
    Status getSensorValues(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::SensorValues *replay) override;
    Status getStateMachineValues(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::StateMachineValues *replay) override;
    // Custom helper function
    void setStatusReply(autoklav::Status *replay, int code);
};
}

Status AutoklavServiceImpl::getStatus(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Status *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    setStatusReply(replay, 0);
    return Status::OK;
}

Status AutoklavServiceImpl::getVariables(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Variables *replay)
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

Status AutoklavServiceImpl::setVariable(grpc::ServerContext *context, const autoklav::SetVariable *request, autoklav::Status *replay)
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

Status AutoklavServiceImpl::startProcess(grpc::ServerContext *context, const autoklav::StartProcessRequest *request, autoklav::Status *replay)
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

Status AutoklavServiceImpl::stopProcess(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::Status *replay)
{
    Q_UNUSED(context);
    Q_UNUSED(request);

    bool succ = StateMachine::instance().stop();

    setStatusReply(replay, !succ);
    return Status::OK;
}

Status AutoklavServiceImpl::getSensorValues(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::SensorValues *replay)
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

Status AutoklavServiceImpl::getStateMachineValues(grpc::ServerContext *context, const autoklav::Empty *request, autoklav::StateMachineValues *replay)
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

void AutoklavServiceImpl::setStatusReply(autoklav::Status *replay, int code)
{
    replay->set_code(code);
    replay->set_errors(GlobalErrors::getErrors());
    replay->set_errorsstring(GlobalErrors::getErrorsString().join("|").toStdString());
}

void GRpcServer::run()
{
    std::string serverUri("127.0.0.1:50061");
    AutoklavServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(serverUri, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    if (!server) {
        qDebug() << "Creating grpc::Server failed.";
        return;
    }

    qDebug() << "Server listening on " << serverUri;
    server->Wait();
}
