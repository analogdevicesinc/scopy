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

#ifndef ACQPLOT_H
#define ACQPLOT_H

#include "acqplotkind.h"

#include <QList>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVector>

class QWidget;

namespace scopy {
class CursorController;
class CursorSettings;
class PlotAxis;
class PlotLegend;
class PlotWidget;

namespace adc {

class AcqChannel;

// One plot in the manager's dock area: a plot widget and the axes its channels draw
// against.
//
// A QObject rather than a QWidget — the PlotWidget goes straight into the dock the
// manager wraps it in, so a wrapper widget here would add a layout level for nothing.
//
// THE AXIS POOL. Every channel here gets its own X and Y axis, which is only safe
// because axes are recycled rather than destroyed. Qwt gives no removal path:
// PlotWidget's removePlotAxis is commented out as "not supported by Qwt"
// (gui/include/gui/plotwidget.h:73), PlotAxis's constructor registers itself into
// PlotWidget::m_plotAxis and bumps the QwtPlot's axis count while its destructor is
// empty and unregisters nothing, and PlotNavigator has addAxis but no removeAxis. So
// acquireAxis() hands out a hidden axis from a free list or grows the pool, and
// releaseAxis() hides it and puts it back. Nothing is ever deleted, so nobody's stored
// axisId can dangle; reuse is safe because PlotNavigator::addChannel re-registers an
// axisId it had dropped.
//
// The consequence worth knowing: axisPoolSize() rises to the high-water mark of
// simultaneous channels and then stops. A pool that keeps growing while channels are
// added and removed one at a time means a release path was missed.
class AcqPlot : public QObject
{
	Q_OBJECT
public:
	// The widget comes from createPlotWidget(), which always builds one.
	AcqPlot(const QString &name, AcqPlotKind kind, quint32 uuid, QWidget *parent = nullptr);
	~AcqPlot() override;

	// Null once the widget has been destroyed — the plot and its widget are siblings
	// under different parents, so always check before use.
	scopy::PlotWidget *plot() const;
	AcqPlotKind kind() const { return m_kind; }
	quint32 uuid() const { return m_uuid; }
	// Keys this plot's rail row and menu page. "acqplot:<uuid>".
	QString menuId() const;
	QString name() const { return m_name; }
	void setName(const QString &n);

	// The channels drawn here. The manager owns the AcqChannel objects; this list
	// exists so per-plot limits and iteration do not have to filter the manager's.
	QList<AcqChannel *> channels() const { return m_channels; }
	void addChannelRef(AcqChannel *ch);
	void removeChannelRef(AcqChannel *ch);

	// False for a kind whose widget owns its own axes (a waterfall), in which case a
	// channel here must not take a pooled pair.
	bool supportsPerChannelAxes() const;

	// --- the axis pool ---

	// A hidden PlotAxis at `position` (a QwtAxis position), recycled if one is free.
	// The caller borrows it and must hand it back through releaseAxis; it must never
	// be deleted. Null if this plot has no widget.
	scopy::PlotAxis *acquireAxis(int position);

	// Hides `ax` and returns it to the free list for its position. Ignores an axis
	// this pool did not hand out, and a double release — putting one axis in the free
	// list twice would give two live channels the same axis.
	void releaseAxis(scopy::PlotAxis *ax);

	// Total axes ever created by the pool. Diagnostics: this must plateau.
	int axisPoolSize() const { return m_all.size(); }

	// The one axis hosting fixed-height items (digital tracks, annotation bands) on
	// this plot. Created on first use, interval [0,1], invisible: items place
	// themselves in canvas pixels off their own handle, so the axis exists only to host
	// handles and items. Same construction as DigitalTrackManager::setPlot()
	// (src/sim/DigitalTrackManager.cpp:71-80).
	//
	// Deliberately NOT part of the pool: it is shared by every logic track here, so it
	// is created once and never released.
	scopy::PlotAxis *digitalAxis();

	// Next free vertical slot for a fixed-height item, in digitalAxis scale coords.
	// Walks down in 26 px steps (24 px band + 2 px gap) — the pitch DigitalCurveItem
	// and AnnotationCurve both render at.
	double nextDigitalSlot();

	// Samples across the visible width. There is no ramp here any more: the sample-index
	// X source is a real store stream written once by the engine
	// (AcquisitionEngine::indexRampKey()), so a per-plot QVector<float> would be a
	// duplicate of it that every narrow plot had to keep in step.
	int plotSize() const { return m_plotSize; }
	void setPlotSize(int n);

	// --- per-plot view options ---

	// The axis scale labels. One toggle for both sides: PlotWidget stores an X and a Y
	// flag separately but its showAxisLabels()/hideAxisLabels() act on both at once, and
	// every caller in the repo moves them together.
	//
	// Re-applied on channel selection as well as on toggle, because PlotWidget shows the
	// *selected* channel's pair and selectChannel() calls showAxisLabels() itself — so
	// without a stored flag here a selection would resurrect labels the reader turned off.
	bool showLabels() const { return m_showLabels; }
	void setShowLabels(bool on);

	// The floating channel legend. Created on first use: it is a HoverWidget overlay on
	// the canvas, so a plot nobody asks a legend for pays nothing. It tracks the plot's
	// addedChannel/removedChannel signals itself, so it needs no per-channel wiring here.
	bool showLegend() const { return m_showLegend; }
	void setShowLegend(bool on);

	// This plot's own cursors, and the settings page that drives them. Created together
	// on first use, because CursorController installs four draggable handles and a
	// readout overlay whether or not they are visible.
	//
	// One controller per plot rather than one per instrument: PlotCursors binds its
	// handles to one canvas in its constructor and has no retarget path, which is why the
	// old manager-wide cursors() could only ever serve the first plot.
	//
	// Both are children of this object, so they die with the plot. Null only if this plot
	// has no widget.
	scopy::CursorController *cursors();
	// The CursorSettings page for those cursors, parented to `parent` on first call.
	// Null for a plot with no widget.
	scopy::CursorSettings *cursorSettings(QWidget *parent = nullptr);
	bool showCursors() const { return m_showCursors; }
	void setShowCursors(bool on);

public Q_SLOTS:
	void replot();

Q_SIGNALS:
	void nameChanged(QString name);
	// So a settings page built before the toggle changed elsewhere can follow it.
	void showLabelsChanged(bool on);
	void showLegendChanged(bool on);
	void showCursorsChanged(bool on);

private:
	QPointer<scopy::PlotWidget> m_plot;
	const AcqPlotKind m_kind;
	const quint32 m_uuid;
	QString m_name;

	// Vertical stacking pitch in canvas pixels: 24 px band + 2 px gap. Kept in sync with
	// DigitalCurveItem/AnnotationCurve visuals — same constant as
	// src/sim/DigitalTrackManager.cpp:26-29.
	const double m_kSlotPitchPx;

	// Headroom above the first band, as a fraction of the axis span.
	const double m_kTopMargin;

	// The width a plot starts at, before the instrument's plot-window control says
	// otherwise.
	const int m_kDefaultPlotSize;

	QList<AcqChannel *> m_channels; // not owned — the manager owns them

	// Free axes by QwtAxis position, and every axis the pool ever made. m_all only
	// grows; see the class comment for why nothing is deleted.
	QMap<int, QList<scopy::PlotAxis *>> m_free;
	QList<scopy::PlotAxis *> m_all;

	// Parented to this, so Qt frees the QObject — which is as much as is safe.
	scopy::PlotAxis *m_digitalAxis{nullptr};
	double m_nextSlot{0.0};
	bool m_slotInit{false};

	int m_plotSize{0};

	// View options. Labels start on, matching what PlotWidget's own subclasses do in
	// their constructors; the other two start off — a plot that opens with a legend over
	// it and four cursors on it is not what anyone wants.
	bool m_showLabels{true};
	bool m_showLegend{false};
	bool m_showCursors{false};

	// All created on demand and parented to this object. QPointer because the legend is
	// also a child of the plot widget, which can be destroyed first.
	QPointer<scopy::PlotLegend> m_legend;
	QPointer<scopy::CursorController> m_cursors;
	QPointer<scopy::CursorSettings> m_cursorSettings;
};

} // namespace adc
} // namespace scopy

#endif // ACQPLOT_H
