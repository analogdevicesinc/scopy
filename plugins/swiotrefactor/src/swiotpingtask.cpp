#include "swiotpingtask.h"

#include <QDebug>

#include <iioutil/connectionprovider.h>
#include <iioutil/iiocommand/iiodevicegettrigger.h>

using namespace scopy;
using namespace scopy::swiotrefactor;
SwiotPingTask::SwiotPingTask(Connection *conn, QObject *parent)
	: QThread(parent)
{
	c = conn;
}

SwiotPingTask::~SwiotPingTask() {}

void SwiotPingTask::run()
{
	enabled = true;
	if(!c) {
		Q_EMIT pingFailed();
		return;
	}

	auto dev = iio_context_find_device(c->context(), "sw_trig");

	if(dev) {
		Command *getTriggerCommand = new IioDeviceGetTrigger(dev, nullptr);
		connect(getTriggerCommand, &scopy::Command::finished, this, &SwiotPingTask::getTriggerCommandFinished,
			Qt::QueuedConnection);
		c->commandQueue()->enqueue(getTriggerCommand);
	}
}

void SwiotPingTask::getTriggerCommandFinished(scopy::Command *cmd)
{
	IioDeviceGetTrigger *tcmd = dynamic_cast<IioDeviceGetTrigger *>(cmd);
	if(!tcmd) {
		return;
	}
	int ret = tcmd->getReturnCode();
	if(ret >= 0 || ret == -ENOENT) {
		Q_EMIT pingSuccess();
	} else {
		Q_EMIT pingFailed();
	}
}

#include "moc_swiotpingtask.cpp"
