#include "iiodeviceimpl.h"
#include "pluginbase/contextprovider.h"

using namespace adiscope;
void IIODeviceImpl::loadCompatiblePlugins()
{
	auto cp = ContextProvider::GetInstance();

	// Optimization for iio plugins - keep context open while running compatible

	cp->open(m_uri);
	DeviceImpl::loadCompatiblePlugins();
	cp->close(m_uri);
}

