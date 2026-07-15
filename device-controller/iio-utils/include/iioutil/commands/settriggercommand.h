#pragma once

#include "core/command.h"
#include "iioutil/ideviceops.h"

#include <cerrno>

namespace scopy::iio {

class SetTriggerCommand : public Command {
	Q_OBJECT
public:
	SetTriggerCommand(IDeviceOps *ops, DeviceHandle handle, DeviceHandle trigger,
			  QObject *parent = nullptr)
		: Command(SetTrigger, handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
		, m_trigger(trigger)
	{}

	void execute() override
	{
		Q_EMIT started(this);
		if(!m_cancelled) {
			m_result = m_ops->setTrigger(m_handle, m_trigger);
		}
		Q_EMIT finished(this);
	}

	Result<void> result() const { return m_result; }

	QString toString() const override
	{
		return QStringLiteral("SetTrigger(device=%1, trigger=%2)")
			.arg(quintptr(m_handle.ptr))
			.arg(quintptr(m_trigger.ptr));
	}

private:
	IDeviceOps *m_ops;
	DeviceHandle m_handle;
	DeviceHandle m_trigger;
	Result<void> m_result{Unexpected{Error{-ENODATA, QStringLiteral("command not executed")}}};
};

} // namespace scopy::iio
