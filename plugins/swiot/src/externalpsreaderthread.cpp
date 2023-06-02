#include "externalpsreaderthread.h"
#include "src/swiot_logging_categories.h"

#include <utility>

using namespace scopy::swiot;

ExternalPsReaderThread::ExternalPsReaderThread(struct iio_device* device, QString attribute, QObject *parent) :
	QThread(parent),
	m_device(device),
	m_attribute(std::move(attribute))
{

}

void ExternalPsReaderThread::run() {
	bool value = false;
	ssize_t res = iio_device_attr_read_bool(m_device, m_attribute.toStdString().c_str(), &value);
	if (res < 0) {
		qCritical(CAT_SWIOT) << "Error, could not read ext_psu attribute from swiot device, error code" << res;
	} else {
		qCritical(CAT_SWIOT) << "testing ps" << res;

		Q_EMIT hasConnectedPowerSupply(value);
	}
}

#include "moc_externalpsreaderthread.cpp"
