#pragma once

#include "component/ping.h"
#include "iioutil/handles.h"

#include <qcoro/qcorotask.h>

namespace scopy {
class ICmdExecutor;
namespace iio {
class IContextOps;
}
} // namespace scopy

namespace scopy::component::iio {

// IIO reachability leaf: dispatches a PingCommand through the injected executor
// (IContextOps::ping may block on network I/O).
class IIOPing : public Ping
{
	Q_OBJECT
public:
	IIOPing(scopy::iio::IContextOps *ops, scopy::iio::ContextHandle handle, scopy::ICmdExecutor *executor,
		QObject *parent = nullptr)
		: Ping(parent)
		, m_ops(ops)
		, m_handle(handle)
		, m_executor(executor)
	{
	}

	bool checkReachable() override;
	void checkReachableAsync() override;

private:
	QCoro::Task<bool> checkInternal();

	scopy::iio::IContextOps *m_ops;
	scopy::iio::ContextHandle m_handle;
	scopy::ICmdExecutor *m_executor;
};

} // namespace scopy::component::iio
