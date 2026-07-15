#pragma once

#include "core/command.h"
#include "iioutil/ideviceops.h"

#include <cerrno>
#include <cstdint>

namespace scopy::iio {

class RegReadCommand : public Command {
	Q_OBJECT
public:
	RegReadCommand(IDeviceOps *ops, DeviceHandle handle, uint32_t addr, QObject *parent = nullptr)
		: Command(RegRead, handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
		, m_addr(addr)
	{}

	void execute() override
	{
		Q_EMIT started(this);
		if(!m_cancelled) {
			m_result = m_ops->regRead(m_handle, m_addr);
		}
		Q_EMIT finished(this);
	}

	Result<uint32_t> result() const { return m_result; }

	QString toString() const override
	{
		return QStringLiteral("RegRead(handle=%1, addr=0x%2)")
			.arg(quintptr(m_handle.ptr))
			.arg(m_addr, 0, 16);
	}

private:
	IDeviceOps *m_ops;
	DeviceHandle m_handle;
	uint32_t m_addr;
	Result<uint32_t> m_result{Unexpected{Error{-ENODATA, QStringLiteral("command not executed")}}};
};

} // namespace scopy::iio
