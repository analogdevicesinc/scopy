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

#ifndef ACQPLOTMANAGER_H
#define ACQPLOTMANAGER_H

#include <core/acq_engine/DataKey.h>
// Not forward-declared: QPointer needs the complete type, and the shell is what the
// rail rows and menu pages are built on.
#include <gui/instrumenttemplate.h>

#include <QList>
#include <QMap>
#include <QPointer>
#include <QVector>
#include <QWidget>

#include <memory>

class QSplitter;
class QTimer;

namespace scopy {
class CursorController;
class CursorSettings;
class MeasurementsPanel;
class PlotWidget;
class StatsPanel;

namespace acq {
class AcquisitionEngine;
class DataStore;
} // namespace acq

namespace adc {

class AcqChannel;
class AcqChannelRepr;
class AcqPlotRow;

// The instrument's center widget: plot rows, the channels on them, and the pull
// loop that feeds them.
//
// This is the seam between the DataStore and the GUI. The engine says "a cycle
// finished"; the manager asks each channel to read what it needs and marks the view
// dirty; a 16 ms timer repaints. Nothing is pushed down a chain of channel objects,
// and no sample data passes through this class — it only decides *when* to read.
//
// The read itself is decoupled from the repaint on purpose: a cycle can complete far
// faster than a screen refresh, and replotting per cycle burns the GUI thread on
// frames nobody sees. Same structure as SimInstrumentController
// (src/sim/siminstrumentcontroller.cpp:405-425).
class AcqPlotManager : public QWidget
{
	Q_OBJECT
public:
	// Which representation a new channel gets. Auto resolves via
	// DataStore::typeOf(), which returns nullopt until the key has been written once
	// (SampleBuffer::type()) — so channels created at composition time, before the
	// engine has ever run, must name their kind. Auto is only meaningful for a channel
	// created after the key has been written at least once.
	enum class ReprKind
	{
		Auto,
		Curve,
		Digital,
		Waterfall,
		Annotations
	};

	AcqPlotManager(scopy::acq::DataStore *store, scopy::acq::AcquisitionEngine *engine, InstrumentTemplate *shell,
		       QWidget *parent = nullptr);
	~AcqPlotManager() override;

	// Returns null when the named kind cannot read the key: a Curve or Digital on an
	// annotation stream, where every numeric conversion returns empty and the channel
	// would silently draw nothing. Auto never hits this — it resolves annotations to
	// ReprKind::Annotations.
	AcqChannel *addChannel(const scopy::acq::DataKey &key, const QString &name, const QColor &color,
			       ReprKind kind = ReprKind::Auto);
	void removeChannel(AcqChannel *ch);
	QList<AcqChannel *> channels() const { return m_channels; }

	int plotSize() const { return m_plotSize; }

	// Row 0, shared by every repr that draws an item rather than owning a plot.
	AcqPlotRow *sharedRow() const { return m_sharedRow; }
	// For CursorController, the panels, and anything else that binds to a PlotWidget.
	PlotWidget *sharedPlot() const;

	// Cursors over row 0. Created on first call, so an instrument that never asks for
	// them pays nothing — CursorController installs four draggable handles and an
	// event filter on the canvas whether or not they are visible. The caller wires the
	// returned controller's setVisible() to whatever toggles it, and parents the
	// CursorSettings widget handed back in `settings` wherever it wants it shown — it
	// is deliberately left unparented rather than pushed into the right menu.
	CursorController *cursors(CursorSettings **settings = nullptr);

	// The measurement and stats readouts, in PS_BOTTOM and PS_TOP. Created on first
	// call and hidden until a label lands in one: both panels hide themselves when
	// their label list empties, so an empty panel must not be shown at all or it
	// reserves plot height for nothing.
	MeasurementsPanel *measurePanel();
	StatsPanel *statsPanel();

	// The timeline horizontal measurements are divided by, applied to every CurveRepr.
	// Unset means 1.0, i.e. measurements come out in samples. Applies to channels
	// created after this call and, so the order of the two does not matter, to those
	// that already exist. Set by the controller, which is the only place that knows the
	// pipeline.
	void setSampleRate(double sr);

public Q_SLOTS:
	// Resizes the index ramp, rescales X, and re-claims every channel: depth is
	// ceil(plotSize/bufferSize), so a wider window needs more chunks.
	void setPlotSize(int n);

	// The engine's buffer size changed. Same reason: halving the buffer doubles the
	// chunks needed to cover one window.
	void onBufferSizeChanged();

	// One acquisition cycle finished. Reads, marks dirty, does NOT replot.
	void onCycleComplete();

	void onStarted();
	void onStopped();

	void replot();

Q_SIGNALS:
	// Payload-free, for panels that only need to know a frame landed. Mirrors
	// PlotManager::newData() (gui/src/plotmanager.cpp:177).
	void newData();

	// The store's key set changed, on the GUI thread. The manager deliberately creates
	// nothing in response — a pluto + FFT pipeline publishes four keys, and four
	// unasked-for curves is worse than an empty plot. Whoever decides which keys become
	// channels listens here.
	void keysAvailable(QList<scopy::acq::DataKey> keys);

private Q_SLOTS:
	void onKeysChanged(QList<scopy::acq::DataKey> keys);

private:
	AcqPlotRow *addRow(PlotWidget *plot, bool exclusive);
	std::unique_ptr<AcqChannelRepr> makeRepr(const scopy::acq::DataKey &key, ReprKind kind);
	void reclaimAll();
	void reclaim(AcqChannel *ch);
	void rebuildIndexRamp();

	// A rail row under "Plots" plus the channel's settings page, both keyed on
	// ch->menuId(). Called from addChannel, so every channel gets its row on one path.
	void registerRail(AcqChannel *ch);
	void unregisterRail(AcqChannel *ch);

	// Binds the channel's repr's measure manager to the two panels, if it has one.
	// Called from addChannel; a repr with nothing to measure (digital, waterfall) is
	// silently skipped.
	void registerMeasurements(AcqChannel *ch);

	// Applies the registered sample rate to a freshly attached channel, if its repr is a
	// curve. Called from addChannel and from setSampleRate, which re-runs it over every
	// existing channel — so it must contain nothing that is per-channel state.
	void applyCurveDefaults(AcqChannel *ch);

	QPointer<scopy::acq::DataStore> m_store;
	QPointer<scopy::acq::AcquisitionEngine> m_engine;
	QPointer<InstrumentTemplate> m_shell;

	QSplitter *m_splitter{nullptr};
	AcqPlotRow *m_sharedRow{nullptr}; // built in the ctor, always row 0
	QList<AcqPlotRow *> m_rows;
	QList<AcqChannel *> m_channels; // parented to this

	// The rail group holding the channel rows, created on the first registerRail so an
	// instrument with no channels shows no empty group. Rows are owned by the shell.
	MenuSectionCollapseWidget *m_railGroup{nullptr};
	QMap<AcqChannel *, MenuControlButton *> m_railRows;
	// The channel's settings page. Kept because removeMenuPage() only unstacks it —
	// MapStackedWidget::remove deletes nothing — so unregisterRail has to.
	QMap<AcqChannel *, QWidget *> m_railPages;

	int m_plotSize{1024};
	// 0..plotSize-1, borrowed by every CurveRepr with no X key. One ramp for all of
	// them; only setPlotSize() resizes it, never a pull().
	QVector<float> m_indexX;

	QTimer *m_frameTimer{nullptr};
	bool m_dirty{false};
	int m_uidCounter{0};

	// All created on demand. Parented into the shell's slots or stack, hence QPointer:
	// the shell can outlive this widget on some teardown orders.
	QPointer<CursorController> m_cursors;
	QPointer<CursorSettings> m_cursorSettings;
	QPointer<MeasurementsPanel> m_measurePanel;
	QPointer<StatsPanel> m_statsPanel;

	// Registered by the controller, applied to every curve.
	double m_sampleRate{1.0};
};

} // namespace adc
} // namespace scopy

#endif // ACQPLOTMANAGER_H
