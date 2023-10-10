#include "swiotpingtask.h"

#include "src/swiot_logging_categories.h"

#include <QDebug>

#include <iioutil/commandqueueprovider.h>
#include <iioutil/iiocommand/iiodevicegettrigger.h>

using namespace scopy;
using namespace scopy::swiot;
SwiotPingTask::SwiotPingTask(iio_context *c, QObject *parent)
	: QThread(parent)
	, c(c)
{}

SwiotPingTask::~SwiotPingTask() {}

void SwiotPingTask::run()
{

	enabled = true;
	CommandQueue *commandQueue = CommandQueueProvider::GetInstance()->open(c);
	if(!commandQueue || !c) {
		Q_EMIT pingFailed();
		return;
	}

	auto dev = iio_context_find_device(c, "sw_trig");

	if(dev) {
		Command *getTriggerCommand = new IioDeviceGetTrigger(dev, nullptr);
		connect(getTriggerCommand, &scopy::Command::finished, this, &SwiotPingTask::getTriggerCommandFinished,
			Qt::QueuedConnection);
		commandQueue->enqueue(getTriggerCommand);
	} else {
		CommandQueueProvider::GetInstance()->close(c);
	}
}

void SwiotPingTask::getTriggerCommandFinished(scopy::Command *cmd)
{
	IioDeviceGetTrigger *tcmd = dynamic_cast<IioDeviceGetTrigger *>(cmd);
	if(!tcmd) {
		CommandQueueProvider::GetInstance()->close(c);
		return;
	}
	int ret = tcmd->getReturnCode();
	if(ret >= 0 || ret == -ENOENT) {
		Q_EMIT pingSuccess();
	} else {
		Q_EMIT pingFailed();
	}
	CommandQueueProvider::GetInstance()->close(c);
}

#include "moc_swiotpingtask.cpp"
