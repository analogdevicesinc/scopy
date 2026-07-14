#pragma once

#include "scopy-core_export.h"

#include "AcquisitionError.h"
#include "DataStore.h"

#include <atomic>
#include <stdexcept>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

class QWidget;

namespace scopy {
namespace acq {

class AcquisitionEngine;

class SCOPY_CORE_EXPORT SourceBlock : public QObject
{
	Q_OBJECT
public:
	explicit SourceBlock(const QString &id, QObject *parent = nullptr);
	virtual ~SourceBlock() = default;

	virtual void acquire(DataStore *store) = 0;

	virtual void onStart();
	virtual void onStop();

	void        setBufferSize(std::size_t size);
	std::size_t bufferSize() const;

	void           enableChannel(const QString &channelId, bool en);
	void           removeChannel(const QString &channelId);
	void           disableAllChannels();
	bool           isChannelEnabled(const QString &channelId) const;
	QList<QString> enabledChannels() const;
	QList<QString> channelIds() const { return m_channels.keys(); }

	const QString &id() const { return m_id; }

	bool isEnabled() const { return m_sourceEnabled.load(std::memory_order_relaxed); }
	void setEnabled(bool en);

	virtual QWidget *createSettingsWidget(QWidget *parent = nullptr);

	// All diagnostics produced by this block must go through report(). The
	// engine multiplexes them into its error(severity, id, message) signal.
	// Parent the block to the AcquisitionEngine so report() can find it;
	// blocks without an engine parent silently drop messages. Throwing from
	// onStart()/acquire() is equivalent to report(Critical, what()) plus
	// aborting the cycle — the engine catches and re-emits.
	void report(AcquisitionError::Severity sev, const QString &msg) const;

Q_SIGNALS:
	void channelsChanged();
	void enabledChanged(bool en);
	// Fired when an individual channel's enable state toggles. Not fired
	// on first-add (channelsChanged covers that). Fine-grained hook for
	// UI consumers (e.g. show/hide a plot track) that don't want to
	// re-read the full channel map on every change.
	void channelEnabledChanged(const QString &channelId, bool en);

protected:
	std::size_t         m_bufferSize{1024};
	QMap<QString, bool> m_channels;
	std::atomic<bool>   m_stopRequested{false};
	std::atomic<bool>   m_sourceEnabled{true};
	QString             m_id;
};

} // namespace acq
} // namespace scopy
