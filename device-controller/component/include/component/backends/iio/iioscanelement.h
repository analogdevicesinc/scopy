#pragma once

#include "core/result.h"
#include "iioutil/handles.h"

#include <QObject>
#include <QString>
#include <QUuid>

#include <qcoro/qcorotask.h>

namespace scopy {
class Command;
class ICmdExecutor;
namespace iio {
class IChannelOps;
}
} // namespace scopy

namespace scopy::component::iio {

// Scan/enable aspect of one streamable channel. Single source of truth for a
// channel's membership in its stream's mask. Concrete IIO-only (no base yet).
class IIOScanElement : public QObject
{
	Q_OBJECT
	Q_PROPERTY(long index READ index CONSTANT)
	Q_PROPERTY(bool isOutput READ isOutput CONSTANT)
public:
	IIOScanElement(scopy::iio::IChannelOps *ops, scopy::iio::ChannelHandle handle, scopy::iio::ChannelsMaskHandle mask,
		       scopy::ICmdExecutor *executor, QObject *parent = nullptr);

	long index() const { return m_index; }
	QString name() const { return m_name; }
	bool isOutput() const { return m_isOutput; }
	scopy::iio::ChannelHandle channelHandle() const { return m_handle; }

	void setIndex(long index)
	{
		m_index = index;
		refreshObjectName();
	}
	void setName(const QString &name)
	{
		m_name = name;
		refreshObjectName();
	}
	void setIsOutput(bool isOutput) { m_isOutput = isOutput; }

	Result<void> enable(bool en);
	Q_INVOKABLE QUuid enableAsync(bool en);
	bool isEnabled() const;

Q_SIGNALS:
	void enabledChanged(bool en);
	void enableFailed(const scopy::Error &error);

private:
	void refreshObjectName() { setObjectName(m_name + QString::number(m_index)); }
	scopy::Command *makeCommand(bool en);
	QCoro::Task<Result<void>> enableInternal(scopy::Command *cmd, bool en);

	scopy::iio::IChannelOps *m_ops;
	scopy::iio::ChannelHandle m_handle;
	scopy::iio::ChannelsMaskHandle m_mask;
	scopy::ICmdExecutor *m_executor;
	long m_index = -1;
	QString m_name;
	bool m_isOutput = false;
};

} // namespace scopy::component::iio
