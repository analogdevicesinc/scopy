#include "iiodeviceimpl.h"

#include "iioutil/contextprovider.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_IIO_DEVICEIMPL, "IIODevice")

using namespace scopy;
void IIODeviceImpl::init()
{
	auto cp = ContextProvider::GetInstance();

	// Optimization for iio plugins - keep context open while running compatible

	cp->open(m_param);
	DeviceImpl::init();
	cp->close(m_param);
}

bool IIODeviceImpl::verify()
{
	iio_context *ctx = ContextProvider::GetInstance()->open(m_param);
	if(!ctx) {
		return false;
	}
	ContextProvider::GetInstance()->close(m_param);
	return true;
}

QMap<QString, QString> IIODeviceImpl::readDeviceInfo()
{
	QMap<QString, QString> contextAttributes;
	iio_context *ctx = ContextProvider::GetInstance()->open(m_param);
	if(!ctx) {
		qWarning(CAT_IIO_DEVICEIMPL) << "Cannot read the device info! (unavailable context)";
	} else {
		for(int i = 0; i < iio_context_get_attrs_count(ctx); i++) {
			const char *name;
			const char *value;
			int ret = iio_context_get_attr(ctx, i, &name, &value);
			if(ret != 0)
				continue;
			contextAttributes[name] = value;
		}
		ContextProvider::GetInstance()->close(m_param);
	}

	return contextAttributes;
}
