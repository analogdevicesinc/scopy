/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "scopy-core_export.h"

#include "acquisitionerror.h"
#include "datakey.h"
#include "samplebuffer.h"

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

class DataStore;

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
	// samplebuffer.h.
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

	// Reports only when `msg` differs from the last message passed here, so a
	// condition that persists across cycles is reported once instead of at cycle
	// rate. The engine has the same thing for its own warnings
	// (AcquisitionEngine::reportWarningOnce) — this is it for blocks, which until
	// now could only report unconditionally.
	//
	// Use this for any report on the per-cycle path whose cause is steady state:
	// a misconfiguration ("no rawKeys set"), a missing input, a wrong stream
	// type. Those re-fire every frame for as long as the condition holds, and at
	// continuous-mode rates the queued signals pile up faster than the GUI thread
	// can drain them.
	//
	// The message is the identity: a report whose text carries per-frame values
	// never matches the previous one and so is not deduped. Keep varying detail
	// out of messages used here. Clears when the message changes, so a condition
	// that goes away and returns is reported again.
	void reportOnce(AcquisitionError::Severity sev, const QString &msg) const;

	// False when a report at this severity would be discarded. Check it before
	// building diagnostic strings in per-cycle paths.
	bool wantsReport(AcquisitionError::Severity sev) const;

Q_SIGNALS:
	void enabledChanged(bool en);

protected:
	// Stacks the base enable checkbox above `own` in a fresh container.
	QWidget *withBaseSettings(QWidget *own, QWidget *parent);

	// The store this block's engine writes to, or null when the block has no
	// engine parent. Borrowed, never owned. Lets a block reach the store from
	// its own constructor rather than wait for a host to inject it — which is
	// what makes a settings widget needing the store self-buildable.
	DataStore *engineStore() const;

	QString m_name;
	std::atomic<bool> m_enabled{true};

private:
	// Weak: the hosting layout owns the widget. Main thread only — nothing in
	// the cycle path touches it.
	QPointer<QWidget> m_settingsWidget;

	// Last message reportOnce() emitted. Mutable because report() is const and
	// callers treat reporting as a const operation. Touched only from the worker
	// thread, on the cycle path.
	mutable QString m_lastReportOnce;
};

} // namespace acq
} // namespace scopy
