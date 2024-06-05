#include "iiopingtask.h"

#include <QDebug>

using namespace scopy;
IIOPingTask::IIOPingTask(iio_context *c, QObject *parent)
	: QThread(parent)
	, c(c)
	, m_callback(nullptr)
{}

IIOPingTask::~IIOPingTask() {}

void IIOPingTask::run()
{
	bool ret;
	enabled = true;
	if (m_callback) {
		ret = m_callback();
	} else {
		ret = ping(c);
	}

	if(isInterruptionRequested())
		return;
	if(ret)
		Q_EMIT pingSuccess();
	else
		Q_EMIT pingFailed();
}

void IIOPingTask::setCallback(const std::function<bool()> callback)
{
	m_callback = callback;
}

bool IIOPingTask::ping(iio_context *ctx)
{
	auto dev = iio_context_get_device(ctx, 0);
	const iio_device *test_device = nullptr;

	int ret = iio_device_get_trigger(dev, &test_device);

	if(ret < 0 && ret != -ENOENT) {
		return false;
	}
	return true;
}

#include "moc_iiopingtask.cpp"
