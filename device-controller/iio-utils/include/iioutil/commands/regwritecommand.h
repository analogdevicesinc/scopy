#pragma once

#include "core/command.h"
#include "iioutil/ideviceops.h"

#include <cerrno>

namespace scopy::iio {

class RegWriteCommand : public Command {
	Q_OBJECT
public:
	RegWriteCommand(IDeviceOps *ops, DeviceHandle handle, uint32_t addr, uint32_t val,
			QObject *parent = nullptr)
		: Command(RegWrite, handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
		, m_addr(addr)
		, m_val(val)
	{}

	void execute() override
	{
		Q_EMIT started(this);
		if(!m_cancelled) {
			m_result = m_ops->regWrite(m_handle, m_addr, m_val);
		}
		Q_EMIT finished(this);
	}

	Result<void> result() const { return m_result; }

	QString toString() const override
	{
		return QStringLiteral("RegWrite(handle=%1, addr=0x%2, val=0x%3)")
			.arg(quintptr(m_handle.ptr))
			.arg(m_addr, 0, 16)
			.arg(m_val, 0, 16);
	}

private:
	IDeviceOps *m_ops;
	DeviceHandle m_handle;
	uint32_t m_addr;
	uint32_t m_val;
	Result<void> m_result{Unexpected{Error{-ENODATA, QStringLiteral("command not executed")}}};
};

} // namespace scopy::iio
