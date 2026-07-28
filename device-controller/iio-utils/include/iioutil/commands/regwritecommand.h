#pragma once

#include "core/resultcommand.h"
#include "iioutil/ideviceops.h"

#include <cerrno>

namespace scopy::iio {

class RegWriteCommand : public ResultCommand<void>
{
	Q_OBJECT
public:
	RegWriteCommand(IDeviceOps *ops, DeviceHandle handle, uint32_t addr, uint32_t val, QObject *parent = nullptr)
		: ResultCommand(handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
		, m_addr(addr)
		, m_val(val)
	{
	}

protected:
	void run() override { setResult(m_ops->regWrite(m_handle, m_addr, m_val)); }

private:
	IDeviceOps *m_ops;
	DeviceHandle m_handle;
	uint32_t m_addr;
	uint32_t m_val;
};

} // namespace scopy::iio
