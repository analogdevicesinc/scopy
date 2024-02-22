#include "iiodeviceimpl.h"

#include "iioutil/connectionprovider.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_IIO_DEVICEIMPL, "IIODevice")

using namespace scopy;
void IIODeviceImpl::init()
{
	auto cp = ConnectionProvider::GetInstance();

	// Optimization for iio plugins - keep context open while running compatible

	cp->open(m_param);
	DeviceImpl::init();
	cp->close(m_param);
}

bool IIODeviceImpl::verify()
{
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(!conn) {
		return false;
	}
	ConnectionProvider::GetInstance()->close(m_param);
	return true;
}

QMap<QString, QString> IIODeviceImpl::readDeviceInfo()
{
	QMap<QString, QString> contextAttributes;
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(!conn) {
		qWarning(CAT_IIO_DEVICEIMPL) << "Cannot read the device info! (unavailable context)";
	} else {
		for(int i = 0; i < iio_context_get_attrs_count(conn->context()); i++) {
			const char *name;
			const char *value;
			int ret = iio_context_get_attr(conn->context(), i, &name, &value);
			if(ret != 0)
				continue;
			contextAttributes[name] = value;
		}
		ConnectionProvider::GetInstance()->close(m_param);
	}

	return contextAttributes;
}
