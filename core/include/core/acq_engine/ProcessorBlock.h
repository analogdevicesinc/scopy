#pragma once

#include "scopy-core_export.h"

#include "AcquisitionError.h"
#include "DataKey.h"
#include "DataStore.h"

#include <atomic>
#include <stdexcept>
#include <QList>
#include <QObject>
#include <QString>

class QWidget;

namespace scopy {
namespace acq {

class AcquisitionEngine;

class SCOPY_CORE_EXPORT ProcessorBlock : public QObject
{
	Q_OBJECT
public:
	explicit ProcessorBlock(const QString &name, QObject *parent = nullptr);
	virtual ~ProcessorBlock() = default;

	virtual const QList<DataKey> &watchedKeys() const { return m_watchedKeys; }
	void setWatchedKeys(const QList<DataKey> &keys) { m_watchedKeys = keys; }

	virtual void process(DataStore *store) = 0;
	virtual void reset() {}

	bool           isEnabled() const { return m_enabled.load(std::memory_order_relaxed); }
	void           setEnabled(bool en) { m_enabled.store(en, std::memory_order_relaxed); }
	const QString &name() const { return m_name; }

	virtual QWidget *createSettingsWidget(QWidget *parent = nullptr);

	// All diagnostics produced by this block must go through report(). The
	// engine multiplexes them into its error(severity, id, message) signal.
	// Parent the block to the AcquisitionEngine so report() can find it;
	// blocks without an engine parent silently drop messages. Throwing from
	// process() is equivalent to report(Critical, what()) plus aborting the
	// cycle — the engine catches and re-emits.
	void report(AcquisitionError::Severity sev, const QString &msg) const;

protected:
	QString           m_name;
	std::atomic<bool> m_enabled{true};
	QList<DataKey>    m_watchedKeys;
};

} // namespace acq
} // namespace scopy
