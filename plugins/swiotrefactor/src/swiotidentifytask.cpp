#include "swiotidentifytask.h"

#include "swiot_logging_categories.h"

#include <iio.h>

#include <iioutil/connectionprovider.h>
#include <iioutil/iiocommand/iiodeviceattributewrite.h>

using namespace scopy::swiotrefactor;
SwiotIdentifyTask::SwiotIdentifyTask(QString uri, QObject *parent)
	: QThread(parent)
	, m_uri(uri)
	, m_conn(nullptr)
{
	m_conn = ConnectionProvider::open(m_uri);
	if(!m_conn) {
		qDebug(CAT_SWIOT) << "Error, no context available for the identify task.";
	}
	connect(m_conn, &Connection::aboutToBeDestroyed, this, [=, this]() { m_conn = nullptr; });
}

SwiotIdentifyTask::~SwiotIdentifyTask()
{
	if(m_conn) {
		m_conn = nullptr;
		ConnectionProvider::close(m_uri);
	}
}

void SwiotIdentifyTask::run()
{
	if(!m_conn) {
		return;
	}

	if(isInterruptionRequested()) {
		return;
	}
	iio_device *swiotDevice = iio_context_find_device(m_conn->context(), "swiot");

	if(swiotDevice) {
		IioDeviceAttributeWrite *iioAttrWrite =
			new IioDeviceAttributeWrite(swiotDevice, "identify", "1", nullptr, true);

		connect(
			iioAttrWrite, &scopy::Command::finished, this,
			[=, this](scopy::Command *cmd) {
				IioDeviceAttributeWrite *tcmd = dynamic_cast<IioDeviceAttributeWrite *>(cmd);
				if(!tcmd) {
					return;
				}
				if(tcmd->getReturnCode() < 0) {
					qCritical(CAT_SWIOT) << "Error, could not identify swiot, error code"
							     << tcmd->getReturnCode();
				}
			},
			Qt::QueuedConnection);

		m_conn->commandQueue()->enqueue(iioAttrWrite);
	}
}
