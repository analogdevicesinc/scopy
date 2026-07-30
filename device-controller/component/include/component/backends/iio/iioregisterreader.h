#pragma once

#include "core/result.h"
#include "iioutil/handles.h"

#include <QObject>
#include <QUuid>
#include <cstdint>

#include <qcoro/qcorotask.h>

namespace scopy {
class ICmdExecutor;
namespace iio {
class IDeviceOps;
class RegReadCommand;
}
} // namespace scopy

namespace scopy::component::iio {

// Register-read capability of a device (debug register access). IIO-only, no
// generic base — a register is addressed by number, so it attaches straight to
// the IIODevice (discovered via findChild<IIORegisterReader*>()).
class IIORegisterReader : public QObject
{
	Q_OBJECT
public:
	IIORegisterReader(scopy::iio::IDeviceOps *ops, scopy::iio::DeviceHandle handle, scopy::ICmdExecutor *executor,
			  QObject *parent = nullptr);

	Result<uint32_t> read(uint32_t addr);
	Q_INVOKABLE QUuid readAsync(uint32_t addr);

Q_SIGNALS:
	void readSucceeded(uint32_t addr, uint32_t value);
	void readFailed(const scopy::Error &error);

private:
	QCoro::Task<Result<uint32_t>> readInternal(scopy::iio::RegReadCommand *cmd, uint32_t addr);

	scopy::iio::IDeviceOps *m_ops;
	scopy::iio::DeviceHandle m_handle;
	scopy::ICmdExecutor *m_executor;
};

} // namespace scopy::component::iio
