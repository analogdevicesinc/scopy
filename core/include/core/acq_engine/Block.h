#pragma once

#include "scopy-core_export.h"

#include "AcquisitionError.h"

#include <atomic>
#include <QObject>
#include <QString>

class QWidget;

namespace scopy {
namespace acq {

// Shared base for SourceBlock and ProcessorBlock: identity, enable state,
// diagnostics and the settings-widget scaffold.
//
// Diagnostics go through report(), which the engine multiplexes into its
// error(severity, id, message) signal. A block must be parented to its
// AcquisitionEngine for this to work; blocks without an engine parent drop
// their messages. Throwing from a block's virtual is equivalent to
// report(Critical, what()) plus aborting the cycle.
class SCOPY_CORE_EXPORT Block : public QObject
{
	Q_OBJECT
public:
	explicit Block(const QString &name, QObject *parent = nullptr);

	const QString &name() const { return m_name; }

	bool isEnabled() const { return m_enabled.load(std::memory_order_relaxed); }
	void setEnabled(bool en);

	// Base implementation returns just the enable checkbox. Subclasses that
	// add their own controls should wrap them with withBaseSettings().
	virtual QWidget *createSettingsWidget(QWidget *parent = nullptr);

	void report(AcquisitionError::Severity sev, const QString &msg) const;

	// False when a report at this severity would be discarded. Check it before
	// building diagnostic strings in per-cycle paths.
	bool wantsReport(AcquisitionError::Severity sev) const;

Q_SIGNALS:
	void enabledChanged(bool en);

protected:
	// Stacks the base enable checkbox above `own` in a fresh container.
	QWidget *withBaseSettings(QWidget *own, QWidget *parent);

	QString           m_name;
	std::atomic<bool> m_enabled{true};
};

} // namespace acq
} // namespace scopy
