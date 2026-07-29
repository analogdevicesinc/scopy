#pragma once

#include "component/device.h"
#include "iioutil/handles.h"

namespace scopy::component::iio {

// IIO device identity. Carries the libiio device handle on top of the shared
// Device identity (id/name/label).
class IIODevice : public Device
{
	Q_OBJECT
public:
	explicit IIODevice(QObject *parent = nullptr)
		: Device(parent)
	{
	}

	scopy::iio::DeviceHandle handle() const { return m_handle; }
	void setHandle(scopy::iio::DeviceHandle handle) { m_handle = handle; }

private:
	scopy::iio::DeviceHandle m_handle;
};

} // namespace scopy::component::iio
