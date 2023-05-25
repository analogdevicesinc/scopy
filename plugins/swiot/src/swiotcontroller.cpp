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

QVector<QString> SwiotController::getAd74413rChannelFunctions() {
	iio_device* swiot = iio_context_find_device(m_iioCtx, "swiot");

	auto result = QVector<QString>();
	for (int i = 0; i < 4; ++i) {
		char device[256] = {0};
		std::string deviceAttributeName = "ch" + std::to_string(i) + "_device";
		ssize_t deviceReadResult = iio_device_attr_read(swiot, deviceAttributeName.c_str(), device, 255);
		if (deviceReadResult > 0) {
			if (strcmp(device, "max14906") == 0) {
				result.push_back("high_z");
			} else {
				char function[256] = {0};
				std::string functionAttributeName = "ch" + std::to_string(i) + "_function";
				ssize_t functionReadResult = iio_device_attr_read(swiot, functionAttributeName.c_str(), function, 255);
				if (functionReadResult > 0) {
					result.push_back(QString(function));
				}
			}
		}
	}

	return result;
}
