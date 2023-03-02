#include "iiopingtask.h"
#include <QDebug>

using namespace adiscope;
IIOPingTask::IIOPingTask(iio_context *c ,QObject *parent) : Task(parent), c(c) {}

IIOPingTask::~IIOPingTask() {}

void IIOPingTask::run() {

	bool ret = ping(c);

	if(!enabled)
		return;
	if(ret)
		Q_EMIT pingSuccess();
	else
		Q_EMIT pingFailed();
}

void IIOPingTask::start(QThread::Priority p) { enabled = true; QThread::start(p);}

void IIOPingTask::stop() { enabled = false; }

bool IIOPingTask::ping(iio_context *ctx) {
	auto dev = iio_context_get_device(ctx, 0);
	const iio_device* test_device = nullptr;

	int ret = iio_device_get_trigger(dev, &test_device);

	if (ret < 0 && ret != -ENOENT) {
		return false;
	}
	return true;
}
