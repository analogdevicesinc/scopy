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

#include "acqchannel.h"
// For AcqPlotKind, which is part of addPlot's signature.
#include "acqplotkind.h"

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/DataKey.h>
// The docking interfaces only, never a kddockwidgets header: which backend builds a dock
// is the factories' business (gui/docking/dockablearea.h), and it is decided by both a
// compile-time flag and a runtime preference.
#include <gui/docking/dockableareainterface.h>
#include <gui/docking/dockwrapperinterface.h>
// Not forward-declared: QPointer needs the complete type, and the shell is what the
// rail rows and menu pages are built on.
#include <gui/instrumenttemplate.h>

#include <QList>
#include <QMap>
#include <QPointer>
#include <QWidget>

class QTimer;

namespace scopy {
class CursorSettings;
class MenuCombo;
class PlotWidget;

namespace acq {
class DataStore;
} // namespace acq

namespace adc {

class AcqPlot;

// The instrument's center widget, and the root of a three-level tree:
//
//     AcqPlotManager  — a dock area of plots, the rail group, the pull loop
//     └── AcqPlot     — one plot widget and its axis pool
//         └── AcqChannel — one X source, one Y source, one AcqAxis per side
//
// This is the seam between the DataStore and the GUI. The engine says "a cycle
// finished"; the manager asks each channel to read what it needs and marks the view
// dirty; a 16 ms timer repaints. Nothing is pushed down a chain of channel objects,
// and no sample data passes through this class — it only decides *when* to read.
//
// The read is decoupled from the repaint on purpose: a cycle can complete far faster
// than a screen refresh, and replotting per cycle burns the GUI thread on frames
// nobody sees. Same structure as SimInstrumentController
// (src/sim/siminstrumentcontroller.cpp:405-425).
//
// It knows nothing about any particular representation *or* any particular plot
// widget: channels come from AcqChannelRegistry keyed on a ReprKind, plot widgets from
// createPlotWidget() keyed on an AcqPlotKind. No concrete kind is named here and no
// header of one is included, which is what makes a fifth channel kind or a third plot
// kind one new .cpp and nothing else.
//
// EVERYTHING IS CREATED BY AN EXPLICIT CALL, and now that includes the plots: the
// reader adds a plot from the rail, picks its kind, then adds channels to it choosing
// an X and a Y source per channel. There is no policy, no inference from SampleType
// and no reconcile against the store's key set — a stream nobody asked to draw is not
// drawn. StreamInfo still supplies the *details* of a channel that was asked for (its
// label, unit, colour, sample rate and recommended X source), because those are facts
// the producer owns; it never decides that a channel exists.
//
// AN EMPTY MANAGER IS A VALID STATE. Any plot is deletable, including the last one, so
// nothing here may assume a first plot exists — sharedPlot() answers null when there are
// no plots.
class AcqPlotManager : public QWidget
{
	Q_OBJECT
public:
	// `engine` supplies stream descriptors and the declared-key list; `store` supplies
	// samples. Both are borrowed.
	//
	// The engine is read, never driven: nothing here starts, stops or configures it,
	// and cycle/start/stop still arrive as signals from AcqInstrument, already on the
	// GUI thread. Depth claims are made in samples and converted by the store, so
	// bufferSize() is not read either.
	AcqPlotManager(scopy::acq::DataStore *store, scopy::acq::AcquisitionEngine *engine,
		       InstrumentTemplate *shell, QWidget *parent = nullptr);
	~AcqPlotManager() override;

	// ---- plots ---------------------------------------------------------------

	// A new plot at the bottom of the dock area, its widget built by createPlotWidget()
	// and wrapped in a dock so it carries a title bar and can be dragged, tabbed and
	// resized. Never null.
	AcqPlot *addPlot(const QString &name, AcqPlotKind kind);

	// Deletes the plot, its channels, its rail row and its menu pages. Any plot may go,
	// including the last: an empty manager is a valid state.
	void removePlot(quint32 uuid);

	AcqPlot *plot(quint32 uuid) const;
	QList<AcqPlot *> plots() const { return m_plots; }

	// ---- channels ------------------------------------------------------------

	// Adds a channel on `plot`, drawn as `kind` says, reading `yKey` against `xKey`.
	//
	// An empty `xKey` means "the producer's recommendation if it made one, otherwise
	// the plot's sample-index ramp" — the channel resolves that, not this function.
	//
	// Returns null, with a warning naming the reason, only when `plot` is null or not one
	// of ours, or when nothing has registered `kind`.
	//
	// NOTHING ELSE IS CHECKED. Whether the combination makes sense is the reader's
	// call: a Waterfall channel on a Basic plot draws nothing, a Curve on a Waterfall
	// plot draws over the raster, two identical channels overlap, and a Curve on an
	// annotation stream is an empty curve. This is a debug instrument for developers, so
	// a combination that produces nothing useful is more informative than a refusal in
	// the log — and none of them is a crash.
	//
	// Two channels may share a Y key. That is not a mistake: with X per channel, the
	// same stream against two different X sources is two different curves.
	//
	// `info` overrides what the store holds for the key — for drawing a stream the
	// producer never described, or overriding its label or colour.
	AcqChannel *addChannel(AcqPlot *plot, scopy::acq::ReprKind kind, const scopy::acq::DataKey &yKey,
			       const scopy::acq::DataKey &xKey = scopy::acq::DataKey(),
			       const std::optional<scopy::acq::StreamInfo> &info = std::nullopt);

	void removeChannel(AcqChannel *ch);
	QList<AcqChannel *> channels() const { return m_channels; }

	// ---- geometry ------------------------------------------------------------

	// The default visible width applied to new plots, and to every existing plot by
	// setPlotSize(). Per-plot because plots can be different widths; the manager holds
	// only the default, the plot holds its own width and index ramp.
	int plotSize() const { return m_plotSize; }

	// The visible width a given channel reads under, in samples: its plot's own, since
	// plots can be different widths. There is nothing else to pass — a depth claim is in
	// samples and the DataStore converts, so no buffer size, and the sample-index ramp is
	// a store stream rather than something handed down.
	int plotSizeFor(AcqChannel *ch) const;

	// The first plot, or null when there is none. For CursorController and anything
	// else that binds to a PlotWidget.
	PlotWidget *sharedPlot() const;

	// Cursors are per plot and live on AcqPlot — see AcqPlot::cursors(). There is
	// deliberately no manager-wide accessor: PlotCursors binds its handles to one canvas
	// in its constructor and cannot be retargeted, so a single controller could only ever
	// serve one plot, which is what the old instrument-wide "Cursors" button did.

	// There are deliberately no measurement or stats panels. Measurements are not part of
	// this instrument: nothing here computes one, no channel kind offers a selector for one,
	// and no slot is reserved for a readout. See acqchannel.h for the same note on the
	// channel side.

	// The timeline for channels whose producer declared none — a raw ADC channel,
	// where the source block does not know the device rate. Set by the controller,
	// which is the only thing that knows the pipeline. A channel whose StreamInfo
	// carries a rate ignores this: the producer's own statement wins.
	//
	// Applies to channels created after this call and, so the order of the two does
	// not matter, to those that already exist.
	void setFallbackSampleRate(double sr);

public Q_SLOTS:
	// The visible width in samples, for every plot and for plots added later. Rescales
	// each plot's ramp and re-claims every channel, which asks the store to retain
	// enough for the wider window.
	//
	// There is deliberately no counterpart for the engine's buffer size: claims are
	// registered in samples and the DataStore re-derives the chunk count on every
	// push, so a resized buffer needs nothing from the view.
	void setPlotSize(int n);

	// One acquisition cycle finished. Reads, marks dirty, does NOT replot.
	void onCycleComplete();

	void onStarted();
	void onStopped();

	void replot();

Q_SIGNALS:
	// Payload-free, for panels that only need to know a frame landed. Mirrors
	// PlotManager::newData() (gui/src/plotmanager.cpp:177).
	void newData();

	// The store's key set changed, on the GUI thread. Depth claims have already been
	// re-applied and every source picker refreshed by the time this fires. Nothing is
	// created in response — it is here for a view that wants to report which streams
	// the pipeline is producing.
	void keysAvailable(QList<scopy::acq::DataKey> keys);

	// The widest window any plot draws, in samples. The engine's sample-index ramp has to
	// be at least this long or every plot wider than it reads a short X window and draws
	// a truncated curve.
	//
	// A signal rather than a call, because this class deliberately holds no engine
	// pointer (see the constructor): it states its requirement and the controller, which
	// is the thing that knows the pipeline, applies it.
	void maxWindowSizeChanged(int n);

	void plotAdded(quint32 uuid);
	// Before anything belonging to the plot is torn down, while it is still readable.
	void plotRemoved(quint32 uuid);

	void channelAdded(AcqChannel *ch);
	// Before the channel is destroyed, while it is still safe to read.
	void channelRemoved(AcqChannel *ch);

private Q_SLOTS:
	// Re-applies every channel's depth claim (reset()/remove() erase claims), greys the
	// channels whose key has gone, and repopulates every source picker. Creates
	// nothing.
	void onKeysChanged(QList<scopy::acq::DataKey> keys);

private:
	// What the manager holds for one plot: its rail row (which is also the container its
	// channel rows nest under), the two source pickers on its ADD CHANNEL section, kept
	// so onKeysChanged can repopulate them in place, and the dock its widget lives in.
	//
	// The dock is a raw pointer and not a QPointer: DockWrapperInterface is a plain
	// interface with no QObject base, so QPointer cannot track it. It is destroyed
	// through its QWidget side in removePlot, and otherwise dies with the dock area.
	struct PlotRail
	{
		CollapsableMenuControlButton *row{nullptr};
		QPointer<MenuCombo> yCombo;
		QPointer<MenuCombo> xCombo;
		scopy::DockWrapperInterface *dock{nullptr};
	};

	void reclaimAll();
	void reclaim(AcqChannel *ch);

	// Emits maxWindowSizeChanged with the max over m_plotSize and every plot's own width.
	// Called wherever that max can move: a width change, and a plot appearing or going.
	void announceMaxWindowSize();

	// The "Plots" group, created on first use so an instrument that adds no plot shows
	// no empty group. Also builds the "+ Add plot" row the first time.
	MenuSectionCollapseWidget *railGroup();

	// A rail row for the plot plus its menu page (ADD CHANNEL · Delete plot), keyed on
	// plot->menuId().
	void registerPlotRail(AcqPlot *p);
	void unregisterPlotRail(AcqPlot *p);
	QWidget *createPlotPage(AcqPlot *p, PlotRail &rail);
	// The VIEW section on a plot's page: labels, legend, and the cursors switch with the
	// CursorSettings page it drops down. All per plot, all held by the AcqPlot.
	QWidget *createViewSection(AcqPlot *p, QWidget *parent);

	// The add-plot row and its page. One per manager, built by railGroup().
	void createAddPlotRow(MenuSectionCollapseWidget *group);

	// A rail row under the channel's plot's row plus the channel's settings page, both
	// keyed on ch->menuId(). Called from addChannel, so every channel gets its row on
	// one path.
	void registerRail(AcqChannel *ch);
	void unregisterRail(AcqChannel *ch);

	// Fills `combo` with the available streams — declared ∪ written, sorted — keeping
	// the current selection where it still exists. `withSampleIndex` prepends the
	// sample-index entry, which is what an X picker wants and a Y picker does not.
	// Mirrors AcqAxis::refreshSourceChoices(); see there for why each step is as it is.
	void populateKeyCombo(MenuCombo *combo, bool withSampleIndex) const;
	// The key a source combo currently names, empty for its sample-index entry.
	static scopy::acq::DataKey keyFromCombo(const MenuCombo *combo);

	// ~60 Hz. A cycle can complete far faster than this; the dirty flag collapses the
	// extra cycles into one repaint.
	const int m_kFrameIntervalMs;

	QPointer<scopy::acq::DataStore> m_store;
	// Read for stream descriptors and the declared-key list only; the samples come
	// from the store. Asked on demand rather than cached, so a block that re-declares
	// needs no notification path.
	QPointer<scopy::acq::AcquisitionEngine> m_engine;
	QPointer<InstrumentTemplate> m_shell;

	// The container every plot's dock is added to. Which implementation this is —
	// KDDockWidgets or the plain-layout fallback — is createDockableArea's decision. It
	// is a QWidget parented into this manager's layout, so Qt frees it.
	scopy::DockableAreaInterface *m_dockArea{nullptr};

	QList<AcqPlot *> m_plots;       // owned by this
	QList<AcqChannel *> m_channels; // parented to this

	// The rail group holding the plot rows, and one entry per plot. Rows are owned by
	// the shell.
	MenuSectionCollapseWidget *m_railGroup{nullptr};
	QMap<AcqPlot *, PlotRail> m_plotRails;
	QMap<AcqChannel *, MenuControlButton *> m_railRows;

	// The default plot width, and the only geometry this class holds. Neither the ramp
	// nor a per-plot width lives here any more (see AcqPlot), and neither does the
	// engine's chunk size — that belonged to the depth conversion, which moved into the
	// DataStore.
	int m_plotSize{1024};

	QTimer *m_frameTimer{nullptr};
	bool m_dirty{false};
	int m_uidCounter{0};
	quint32 m_plotUuidCounter{0};

	// Next palette slot for a stream whose producer named no colorIndex. Monotonic
	// rather than a count of live channels: reusing a removed channel's slot would
	// recolour nothing but would make two successive channels on one key look like the
	// same trace.
	int m_nextColorIndex{0};

	// Applied to channels whose producer declared no rate.
	double m_fallbackSampleRate{1.0};
};

} // namespace adc
} // namespace scopy

#endif // ACQPLOTMANAGER_H
