#include "swiotswitchctxtask.h"
#include <iioutil/contextprovider.h>
#include "src/swiot_logging_categories.h"

using namespace scopy::swiot;

SwiotSwitchCtxTask::SwiotSwitchCtxTask(QString uri, bool wasRuntime)
	: QThread()
	, m_uri(uri)
	, m_wasRuntime(wasRuntime)
{}

void SwiotSwitchCtxTask::run()
{
	iio_context *ctx = ContextProvider::GetInstance()->open(m_uri);
	if (!ctx) {
		return;
	}
	if (isInterruptionRequested()) {
		ContextProvider::GetInstance()->close(m_uri);
		return;
	}
	iio_device *swiotDevice = iio_context_find_device(ctx, "swiot");
	if (swiotDevice) {
		char mode[64];
		ssize_t result = iio_device_attr_read(swiotDevice, "mode", mode, 64);
		if (result >= 0) {
			if (m_wasRuntime && (strcmp(mode,"runtime") == 0)) {
				return;
			}

			if (!m_wasRuntime && (strcmp(mode,"config") == 0)) {
				return;
			}
			//need to be changed to CAT_SWIOTPLUGIN
			qDebug(CAT_SWIOT)<<"Context has been changed";
			Q_EMIT contextSwitched();
		}
	}
	ContextProvider::GetInstance()->close(m_uri);
}

