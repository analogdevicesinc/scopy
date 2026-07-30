#include "component/backends/iio/iiotrigger.h"

#include "iioutil/ideviceops.h"

using namespace scopy::component::iio;

IIOTrigger::IIOTrigger(scopy::iio::IDeviceOps *ops, scopy::iio::DeviceHandle dev, QObject *parent)
	: QObject(parent)
	, m_ops(ops)
	, m_dev(dev)
{
	setObjectName(name());
}

QString IIOTrigger::name() const { return m_ops->name(m_dev); }
