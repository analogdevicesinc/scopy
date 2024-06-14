#include "iiopingtask.h"

#include <QDebug>

using namespace scopy;
IIOPingTask::IIOPingTask(iio_context *c, QObject *parent)
	: PingTask(parent)
	, m_ctx(c)
{}

IIOPingTask::~IIOPingTask() {}

void IIOPingTask::run()
{
	bool ret = ping();

	if(isInterruptionRequested())
		return;
	if(ret)
		Q_EMIT pingSuccess();
	else
		Q_EMIT pingFailed();
}

bool IIOPingTask::ping()
{
	auto dev = iio_context_get_device(m_ctx, 0);
	const iio_device *test_device = nullptr;

	int ret = iio_device_get_trigger(dev, &test_device);

	if(ret < 0 && ret != -ENOENT) {
		return false;
	}
	return true;
}

#include "moc_iiopingtask.cpp"
