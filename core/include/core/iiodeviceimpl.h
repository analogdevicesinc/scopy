#ifndef IIODEVICEIMPL_H
#define IIODEVICEIMPL_H

#include "deviceimpl.h"

namespace adiscope {

class SCOPYCORE_EXPORT IIODeviceImpl : public DeviceImpl
{
public:
	explicit IIODeviceImpl(QString param, PluginManager *p ,QObject *parent = nullptr) : DeviceImpl(param, p, "iio", parent) {}
	~IIODeviceImpl() {}

	virtual void loadCompatiblePlugins() override;
};

}

#endif // IIODEVICEIMPL_H
