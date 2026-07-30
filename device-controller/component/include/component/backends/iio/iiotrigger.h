#pragma once

#include "iioutil/handles.h"

#include <QObject>
#include <QString>

namespace scopy::iio {
class IDeviceOps;
}

namespace scopy::component::iio {

// Trigger SOURCE marker: an IIO device where iio_device_is_trigger is true.
// IIO-only, no generic base — pure metadata (name()), no I/O. Carries the device
// handle so an IIOTriggerable can pass it to iio_device_set_trigger. Parented to
// its IIODevice; discovered via findChild<IIOTrigger*>().
class IIOTrigger : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString name READ name CONSTANT)
public:
	IIOTrigger(scopy::iio::IDeviceOps *ops, scopy::iio::DeviceHandle dev, QObject *parent = nullptr);

	QString name() const;
	scopy::iio::DeviceHandle handle() const { return m_dev; }

private:
	scopy::iio::IDeviceOps *m_ops;
	scopy::iio::DeviceHandle m_dev;
};

} // namespace scopy::component::iio
