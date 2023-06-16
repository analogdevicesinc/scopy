#include "swiotcontroller.h"

using namespace scopy;
using namespace scopy::swiot;

SwiotController::SwiotController(QString uri, QObject *parent)
	: QObject(parent), uri(uri)
{
	pingTask = nullptr;
	pingTimer = nullptr;
	identifyTask = nullptr;
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
	pingTask->requestInterruption();
	pingTask->deleteLater();
	pingTimer->deleteLater();
}

void SwiotController::startSwitchContextTask(bool isRuntime)
{
	switchCtxTask = new SwiotSwitchCtxTask(uri, isRuntime);
	switchCtxTimer = new CyclicalTask(switchCtxTask, this);
	connect(switchCtxTask, &SwiotSwitchCtxTask::contextSwitched, this, &SwiotController::contextSwitched);
	switchCtxTimer->start(1000);
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
	connect(extPsTask, &ExternalPsReaderThread::hasConnectedPowerSupply, this, &SwiotController::hasConnectedPowerSupply);
	powerSupplyTimer->start(5000);
}

void SwiotController::stopPowerSupplyTask()
{
	powerSupplyTimer->stop();
	extPsTask->requestInterruption();
	disconnect(extPsTask, &ExternalPsReaderThread::hasConnectedPowerSupply, this, &SwiotController::hasConnectedPowerSupply);
}

void SwiotController::startTemperatureTask() {
	temperatureTask = new SwiotReadTemperatureTask(uri, this);
	temperatureTimer = new CyclicalTask(temperatureTask);
	temperatureTimer->start(1000);
	connect(temperatureTask, &SwiotReadTemperatureTask::newTemperature, this, &SwiotController::readTemperature);
}

void SwiotController::stopTemperatureTask() {
	temperatureTimer->stop();
	temperatureTask->requestInterruption();
	disconnect(temperatureTask, &SwiotReadTemperatureTask::newTemperature, this, &SwiotController::readTemperature);
}

void SwiotController::identify()
{
	if(!identifyTask) {
		identifyTask = new SwiotIdentifyTask(uri);
		identifyTask->start();
		connect(identifyTask,&QThread::finished,this,[=](){
			delete identifyTask;
			identifyTask = nullptr;
		});
	}
}
