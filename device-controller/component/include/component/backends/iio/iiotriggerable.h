#pragma once

#include "core/result.h"
#include "iioutil/handles.h"

#include <QObject>
#include <QString>
#include <QUuid>

#include <qcoro/qcorotask.h>

namespace scopy {
class ICmdExecutor;
namespace iio {
class IDeviceOps;
class SetTriggerCommand;
}
} // namespace scopy

namespace scopy::component::iio {

class IIOTrigger;

// Consumer side of the trigger relationship over one libiio device. IIO-only, no
// generic base. hasTrigger/assignedTriggerName are metadata (direct, no I/O);
// setTrigger/clearTrigger go through the executor via SetTriggerCommand. A null
// source (or clearTrigger) assigns no trigger. Parented to its IIODevice;
// discovered via findChild<IIOTriggerable*>().
class IIOTriggerable : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool hasTrigger READ hasTrigger NOTIFY triggerSucceeded)
public:
	IIOTriggerable(scopy::iio::IDeviceOps *ops, scopy::iio::DeviceHandle dev, scopy::ICmdExecutor *executor,
		       QObject *parent = nullptr);

	bool hasTrigger() const;
	QString assignedTriggerName() const;

	Result<void> setTrigger(IIOTrigger *source);
	Q_INVOKABLE QUuid setTriggerAsync(IIOTrigger *source);
	Result<void> clearTrigger();
	Q_INVOKABLE QUuid clearTriggerAsync();

Q_SIGNALS:
	void triggerSucceeded();
	void triggerFailed(const scopy::Error &error);

private:
	QCoro::Task<Result<void>> setInternal(scopy::iio::SetTriggerCommand *cmd);

	scopy::iio::IDeviceOps *m_ops;
	scopy::iio::DeviceHandle m_dev;
	scopy::ICmdExecutor *m_executor;
};

} // namespace scopy::component::iio
