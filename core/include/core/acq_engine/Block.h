#pragma once

#include "scopy-core_export.h"

#include "AcquisitionError.h"
#include "DataKey.h"
#include "SampleBuffer.h"

#include <atomic>
#include <optional>
#include <QHash>
#include <QList>
#include <QObject>
#include <QPointer>
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

	// Keys this block writes into the DataStore. Empty when the block produces
	// no stream of its own — a trigger only emits fired(). Together with
	// ProcessorBlock::watchedKeys() this is the whole graph: an edge exists
	// wherever one block's outputKeys() appears in another's watchedKeys().
	virtual QList<DataKey> outputKeys() const { return {}; }

	// What one of this block's output streams means and how it should be drawn.
	// outputKeys() says *which* streams exist; this says what they are, and the
	// two are deliberately not two lists: this answers nullopt for anything
	// outside outputKeys(), so there is no second enumeration to drift out of
	// step with the first.
	//
	// nullopt for a key this block does not produce, and equally for one it
	// produces but has nothing to say about — a view treats both the same way,
	// falling back to a default-built descriptor (an unlabelled, unitless curve).
	//
	// Asked for through AcquisitionEngine::streamInfo(), which calls this every
	// time rather than caching. So a view drawing one of these keys has its
	// label, unit, rate, colour and X source available before the first cycle —
	// which is what lets it register a depth claim early enough for the first
	// window to be full depth — and a block whose answer changes needs no
	// notification: return the new value and the next reader sees it.
	//
	// Called from the GUI thread while the worker runs, so an implementation that
	// reads mutable state must take the same lock its setter does. One key per
	// call is what keeps that cheap: a lookup takes only the locks the key it
	// asked about actually needs.
	//
	// Describing does not put anything on a plot. The instrument decides what is
	// drawn, by name, in its own setup code; this only says what the stream is. A
	// key answered with ReprKind::Hidden, or not answered at all, is a stream the
	// block does not consider a trace — a frequency ramp feeding an X axis — and
	// stating that is useful even though nothing acts on it automatically.
	//
	// The representation is the block's own choice. There is no inference from
	// SampleType anywhere in the stack, deliberately — see ReprKind in
	// SampleBuffer.h.
	virtual std::optional<StreamInfo> streamInfo(const DataKey &key) const { return std::nullopt; }

	// The block's one settings widget, built on first call and reused after.
	// Prefer this over createSettingsWidget() everywhere in UI code: two
	// widgets for one block are two views that don't mirror each other's
	// edits. `parent` only applies to the first call, which is what builds it.
	//
	// Ownership stays with the caller's layout; the block only holds a weak
	// reference, so a host is free to delete it and get a fresh one later.
	QWidget *settingsWidget(QWidget *parent = nullptr);

	// Hands the cached widget over to the block, for widgets a host must
	// build itself because the constructor needs arguments this class can't
	// supply (a DataStore pointer, the run state). Call before the first
	// settingsWidget(); a second call is ignored.
	void setSettingsWidget(QWidget *w);

	// Whether settingsWidget() would return an already-built widget. Lets a
	// host skip an empty group box without forcing the widget into existence.
	bool hasSettingsWidget() const { return !m_settingsWidget.isNull(); }

	// Base implementation returns just the enable checkbox. Subclasses that
	// add their own controls should wrap them with withBaseSettings().
	// Every call builds a new widget — go through settingsWidget() instead.
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

private:
	// Weak: the hosting layout owns the widget. Main thread only — nothing in
	// the cycle path touches it.
	QPointer<QWidget> m_settingsWidget;
};

} // namespace acq
} // namespace scopy
