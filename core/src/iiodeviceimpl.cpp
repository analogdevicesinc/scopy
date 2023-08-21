#include "iiodeviceimpl.h"
#include "iioutil/contextprovider.h"

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
	if (!ctx) {
		return false;
	}
	ContextProvider::GetInstance()->close(m_param);
	return true;
}

