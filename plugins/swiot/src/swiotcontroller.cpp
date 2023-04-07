#include "swiotcontroller.h"

using namespace adiscope;
using namespace adiscope::swiot;

SwiotController::SwiotController(QString uri, QObject *parent)
	: QObject(parent), uri(uri)
{
	pingTask = nullptr;
}

SwiotController::~SwiotController()
{

}

void SwiotController::startPingTask()
{
	pingTask = new IIOPingTask(m_iioCtx);
	pingTimer = new CyclicalTask(pingTask, this);
	connect(pingTask,SIGNAL(pingSuccess()),this,SIGNAL(pingSuccess()));
	connect(pingTask,SIGNAL(pingFailed()),this,SIGNAL(pingFailed()));
	pingTimer->start(10000);
}

void SwiotController::stopPingTask()
{
	pingTask->requestInterruption();
	pingTask->deleteLater();
	pingTimer->deleteLater();
}

void SwiotController::startSwitchContextTask()
{
	if (pingTask) {
//		stopPingTask();
	}

	switchCtxTask = new SwiotSwitchCtxTask(uri);
	switchCtxTimer = new CyclicalTask(switchCtxTask, this);
	switchCtxTimer->start(5000);
	connect(switchCtxTask,&SwiotSwitchCtxTask::contextSwitched,this,[=]() {
		stopSwitchContextTask();
		Q_EMIT contextSwitched();
	});
}

void SwiotController::stopSwitchContextTask()
{
	switchCtxTask->requestInterruption();
	switchCtxTask->deleteLater();
	switchCtxTimer->deleteLater();
}

void SwiotController::connectSwiot(iio_context *ctx)
{
	m_iioCtx = ctx;
}

void SwiotController::disconnectSwiot()
{
	m_iioCtx = nullptr;
}
