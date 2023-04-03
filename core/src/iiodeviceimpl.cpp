#include "iiodeviceimpl.h"
#include "iioutil/contextprovider.h"

using namespace adiscope;
void IIODeviceImpl::init()
{
	auto cp = ContextProvider::GetInstance();

	// Optimization for iio plugins - keep context open while running compatible

	cp->open(m_param);
	DeviceImpl::init();
	cp->close(m_param);
}

