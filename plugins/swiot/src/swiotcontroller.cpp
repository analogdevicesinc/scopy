#include "swiotcontroller.h"

using namespace scopy;
using namespace scopy::swiot;

SwiotController::SwiotController(QString uri, QObject *parent)
	: QObject(parent), uri(uri)
{
	pingTask = nullptr;
	pingTimer = nullptr;
}

SwiotController::~SwiotController()
{}

void SwiotController::startPingTask()
{
	pingTask = new IIOPingTask(m_iioCtx);
	pingTimer = new CyclicalTask(pingTask);
	connect(pingTask,SIGNAL(pingSuccess()),this,SIGNAL(pingSuccess()));
	connect(pingTask,SIGNAL(pingFailed()),this,SIGNAL(pingFailed()));
	pingTimer->start(2000);
}

void SwiotController::stopPingTask()
{
	pingTimer->stop();
	pingTask->requestInterruption();
	disconnect(pingTask, SIGNAL(pingSuccess()), this, SIGNAL(pingSuccess()));
	disconnect(pingTask, SIGNAL(pingFailed()), this, SIGNAL(pingFailed()));
}

void SwiotController::startSwitchContextTask()
{
	switchCtxTask = new SwiotSwitchCtxTask(uri);
	switchCtxTimer = new CyclicalTask(switchCtxTask, this);
	switchCtxTimer->start(3000);
	connect(switchCtxTask, &SwiotSwitchCtxTask::contextSwitched, this, &SwiotController::contextSwitched);
}

void SwiotController::stopSwitchContextTask()
{
	switchCtxTimer->stop();
	switchCtxTask->requestInterruption();
	disconnect(switchCtxTask, &SwiotSwitchCtxTask::contextSwitched, this, &SwiotController::contextSwitched);
}

void SwiotController::connectSwiot(iio_context *ctx)
{
	m_iioCtx = ctx;
}

void SwiotController::disconnectSwiot()
{
	m_iioCtx = nullptr;
}

void SwiotController::startPowerSupplyTask(QString attribute)
{
	extPsTask = new ExternalPsReaderThread(uri, attribute);
	powerSupplyTimer = new CyclicalTask(extPsTask);
	powerSupplyTimer->start(5000);
	connect(extPsTask, &ExternalPsReaderThread::hasConnectedPowerSupply, this, &SwiotController::hasConnectedPowerSupply);
}

void SwiotController::stopPowerSupplyTask()
{
	powerSupplyTimer->stop();
	extPsTask->requestInterruption();
	disconnect(extPsTask, &ExternalPsReaderThread::hasConnectedPowerSupply, this, &SwiotController::hasConnectedPowerSupply);
}
