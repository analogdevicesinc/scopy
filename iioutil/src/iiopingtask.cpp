#include "iiopingtask.h"
#include <QDebug>

using namespace adiscope;
IIOPingTask::IIOPingTask(iio_context *c ,QObject *parent) : QThread(parent), c(c) {}

IIOPingTask::~IIOPingTask() {}

void IIOPingTask::run() {

	enabled = true;
	bool ret = ping(c);

	if(isInterruptionRequested())
		return;
	if(ret)
		Q_EMIT pingSuccess();
	else
		Q_EMIT pingFailed();
}

bool IIOPingTask::ping(iio_context *ctx) {
	auto dev = iio_context_get_device(ctx, 0);
	const iio_device* test_device = nullptr;

	int ret = iio_device_get_trigger(dev, &test_device);

	//we accept ret to be EINVAL just for swiot plugin development process
	//iio_device_get_trigger returns EINVAL which leads to a plugin disconnection
	//after the problem is solved, "ret != -EINVAL" will be deleted
	if (ret < 0 && ret != -ENOENT && ret != -EINVAL) {
		return false;
	}
	return true;
}

#include "moc_iiopingtask.cpp"
