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
class RegWriteCommand;
}
} // namespace scopy

namespace scopy::component::iio {

// Register-write capability of a device (debug register access). IIO-only, no
// generic base — addressed by number, attaches straight to the IIODevice
// (discovered via findChild<IIORegisterWriter*>()).
class IIORegisterWriter : public QObject
{
	Q_OBJECT
public:
	IIORegisterWriter(scopy::iio::IDeviceOps *ops, scopy::iio::DeviceHandle handle, scopy::ICmdExecutor *executor,
			  QObject *parent = nullptr);

	Result<void> write(uint32_t addr, uint32_t value);
	Q_INVOKABLE QUuid writeAsync(uint32_t addr, uint32_t value);

Q_SIGNALS:
	void writeSucceeded(uint32_t addr);
	void writeFailed(const scopy::Error &error);

private:
	QCoro::Task<Result<void>> writeInternal(scopy::iio::RegWriteCommand *cmd, uint32_t addr);

	scopy::iio::IDeviceOps *m_ops;
	scopy::iio::DeviceHandle m_handle;
	scopy::ICmdExecutor *m_executor;
};

} // namespace scopy::component::iio
