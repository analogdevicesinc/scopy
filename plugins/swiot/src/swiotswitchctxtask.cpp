#include "swiotswitchctxtask.h"
#include <iioutil/contextprovider.h>
#include <iioutil/commandqueueprovider.h>
#include <iioutil/iiocommand/iiodeviceattributeread.h>
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

	CommandQueue *commandQueue = CommandQueueProvider::GetInstance()->open(ctx);
	if (!commandQueue) {
		ContextProvider::GetInstance()->close(m_uri);
		return;
	}

	iio_device *swiotDevice = iio_context_find_device(ctx, "swiot");
	if (swiotDevice) {
		IioDeviceAttributeRead *iioAttrRead = new IioDeviceAttributeRead(swiotDevice, "mode", commandQueue, true);

		connect(iioAttrRead, &scopy::Command::finished, this, [=, this] (scopy::Command *cmd) {
			IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead*>(cmd);
			if (!tcmd) {
				CommandQueueProvider::GetInstance()->close(ctx);
				ContextProvider::GetInstance()->close(m_uri);
				return;
			}
			char *readMode = tcmd->getResult();
			if (tcmd->getReturnCode() >= 0) {
				if ((m_wasRuntime && (strcmp(readMode, "config") == 0)) ||
				    (!m_wasRuntime && (strcmp(readMode, "runtime") == 0))) {
					qDebug(CAT_SWIOT)<<"Context has been changed";
					Q_EMIT contextSwitched();
				}
			}
			CommandQueueProvider::GetInstance()->close(ctx);
			ContextProvider::GetInstance()->close(m_uri);
		}, Qt::QueuedConnection);

		commandQueue->enqueue(iioAttrRead);
	} else {
		CommandQueueProvider::GetInstance()->close(ctx);
		ContextProvider::GetInstance()->close(m_uri);
	}
}

