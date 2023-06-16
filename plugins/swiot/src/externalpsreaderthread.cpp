#include "externalpsreaderthread.h"
#include <iioutil/contextprovider.h>
#include <iioutil/commandqueueprovider.h>
#include <iioutil/iiocommand/iiodeviceattributeread.h>
#include "src/swiot_logging_categories.h"
#include <utility>

using namespace scopy::swiot;

ExternalPsReaderThread::ExternalPsReaderThread(QString uri, QString attr)
	: QThread()
	, m_uri(uri)
	, m_attribute(attr)
{
}

void ExternalPsReaderThread::run() {
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
		IioDeviceAttributeRead *iioAttrRead = new IioDeviceAttributeRead(swiotDevice, m_attribute.toStdString().c_str(), commandQueue, true);

		connect(iioAttrRead, &scopy::Command::finished, this, [=, this](scopy::Command *cmd) {
			IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead*>(cmd);
			if (!tcmd) {
				CommandQueueProvider::GetInstance()->close(ctx);
				ContextProvider::GetInstance()->close(m_uri);
				return;
			}
			if (tcmd->getReturnCode() >= 0) {
				char *extPsu = tcmd->getResult();
				bool ok = false;
				bool extPsuValue = QString(extPsu).toInt(&ok);
				if (ok) {
					Q_EMIT hasConnectedPowerSupply(extPsuValue);
				}
			} else {
				qCritical(CAT_SWIOT) << "Error, could not read ext_psu attribute from swiot device, error code" << tcmd->getReturnCode();
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

#include "moc_externalpsreaderthread.cpp"
