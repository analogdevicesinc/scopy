#pragma once

#include "core/resultcommand.h"
#include "iioutil/ideviceops.h"

#include <cerrno>

namespace scopy::iio {

class SetTriggerCommand : public ResultCommand<void>
{
	Q_OBJECT
public:
	SetTriggerCommand(IDeviceOps *ops, DeviceHandle handle, DeviceHandle trigger, QObject *parent = nullptr)
		: ResultCommand(handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
		, m_trigger(trigger)
	{
	}

protected:
	void run() override { setResult(m_ops->setTrigger(m_handle, m_trigger)); }

private:
	IDeviceOps *m_ops;
	DeviceHandle m_handle;
	DeviceHandle m_trigger;
};

} // namespace scopy::iio
