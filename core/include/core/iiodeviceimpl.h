#ifndef IIODEVICEIMPL_H
#define IIODEVICEIMPL_H

#include "deviceimpl.h"

namespace adiscope {

class SCOPYCORE_EXPORT IIODeviceImpl : public DeviceImpl
{
public:
	explicit IIODeviceImpl(QString uri, PluginManager *p ,QObject *parent = nullptr) : DeviceImpl(uri, p, "iio", parent) {}
	~IIODeviceImpl() {}

	virtual void loadCompatiblePlugins() override;
};

}

#endif // IIODEVICEIMPL_H
