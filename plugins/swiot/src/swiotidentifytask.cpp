#include "swiotidentifytask.h"
#include <iio.h>
#include <iioutil/contextprovider.h>
#include <iioutil/commandqueueprovider.h>
#include <iioutil/iiocommand/iiodeviceattributewrite.h>
#include "src/swiot_logging_categories.h"

using namespace scopy::swiot;
SwiotIdentifyTask::SwiotIdentifyTask(QString uri, QObject *parent) : QThread(parent), m_uri(uri) {}

SwiotIdentifyTask::~SwiotIdentifyTask() {}

void SwiotIdentifyTask::run() {
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
		IioDeviceAttributeWrite *iioAttrWrite = new IioDeviceAttributeWrite(swiotDevice, "identify", "1", nullptr, true);

		connect(iioAttrWrite, &scopy::Command::finished, this, [=, this](scopy::Command *cmd) {
			IioDeviceAttributeWrite *tcmd = dynamic_cast<IioDeviceAttributeWrite*>(cmd);
			if (!tcmd) {
				CommandQueueProvider::GetInstance()->close(ctx);
				ContextProvider::GetInstance()->close(m_uri);
				return;
			}
			if (tcmd->getReturnCode() >= 0) {
				Q_EMIT hasPerformedIdentification();
			} else {
				qCritical(CAT_SWIOT) << "Error, could not identify swiot, error code" << tcmd->getReturnCode();
			}
			CommandQueueProvider::GetInstance()->close(ctx);
			ContextProvider::GetInstance()->close(m_uri);
		}, Qt::QueuedConnection);

		commandQueue->enqueue(iioAttrWrite);
	} else {
		CommandQueueProvider::GetInstance()->close(ctx);
		ContextProvider::GetInstance()->close(m_uri);
	}
}
