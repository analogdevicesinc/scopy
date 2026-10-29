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

	// Keys this block writes into the DataStore. Together with
	// ProcessorBlock::watchedKeys() this is the whole graph.
	virtual QList<DataKey> outputKeys() const { return {}; }

	// What one of this block's outputs means and how to draw it; nullopt for a key it
	// does not produce or has nothing to say about (a view then uses a default, Hidden
	// descriptor). Asked through AcquisitionEngine::streamInfo() on every call, from the
	// GUI thread while the worker runs, so an implementation reading mutable state must
	// take the lock its setter takes.
	virtual std::optional<StreamInfo> streamInfo(const DataKey &key) const { return std::nullopt; }

	// The block's one settings widget, built on first call and reused after, so every
	// view of the block edits the same widget. `parent` applies to the first call.
	// Ownership stays with the caller's layout; a deleted widget is rebuilt next call.
	QWidget *settingsWidget(QWidget *parent = nullptr);

	// Installs a host-built widget as the cached one, for pages that need more than the
	// block to build. Call before the first settingsWidget(); later calls are ignored.
	void setSettingsWidget(QWidget *w);

	// Whether a widget is already built, without forcing one into existence.
	bool hasSettingsWidget() const { return !m_settingsWidget.isNull(); }

	// Base implementation returns just the enable checkbox. Subclasses that
	// add their own controls should wrap them with withBaseSettings().
	// Every call builds a new widget — go through settingsWidget() instead.
	virtual QWidget *createSettingsWidget(QWidget *parent = nullptr);

	void report(AcquisitionError::Severity sev, const QString &msg) const;

	// report(), but only when `msg` differs from the last message passed here, so a
	// steady-state condition on the per-cycle path is reported once, not at cycle rate.
	// The message is the identity: keep per-frame values out of it.
	void reportOnce(AcquisitionError::Severity sev, const QString &msg) const;

	// False when a report at this severity would be discarded. Check it before
	// building diagnostic strings in per-cycle paths.
	bool wantsReport(AcquisitionError::Severity sev) const;

	// Called by AcquisitionEngine::stop() on the GUI thread, before it joins the
	// worker. Must be thread-safe and should make any blocking call on the worker
	// return promptly. The default does nothing.
	virtual void requestStop() {}

Q_SIGNALS:
	void enabledChanged(bool en);

protected:
	// Stacks the base enable checkbox above `own` in a fresh container.
	QWidget *withBaseSettings(QWidget *own, QWidget *parent);

	// The engine's store, or null without an engine parent. Borrowed.
	DataStore *engineStore() const;

	QString m_name;
	std::atomic<bool> m_enabled{true};

private:
	// Weak: the hosting layout owns the widget. GUI thread only.
	QPointer<QWidget> m_settingsWidget;

	// Worker thread only (the per-cycle path).
	mutable QString m_lastReportOnce;
};

} // namespace acq
} // namespace scopy
