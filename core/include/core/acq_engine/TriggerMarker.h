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

#ifndef TRIGGERMARKER_H
#define TRIGGERMARKER_H

#include "scopy-core_export.h"

#include <QMetaObject>
#include <QObject>
#include <QPointer>

namespace scopy {
class PlotAxis;
class PlotAxisHandle;
class PlotWidget;

namespace acq {

class TriggerProcessor;

// The trigger's draggable bar on a plot. A companion to TriggerProcessor rather than part of
// it: process() runs on the acquisition worker thread and must not touch a widget.
//
// Two modes, taken from the processor:
//
//   sample-specific — the bar names the sample the trigger fires on. The axis does not move.
//
//   scanning        — the bar is the trigger *position*. Dragging it sets the pre/post split
//                     and pans the axis so the anchored sample stays under the bar, which is
//                     what makes the waveform slide with the bar.
//
// Nothing here owns the axis: it is shared with every channel drawing against that source
// and outlives the marker, which reads and pans it but never frees it.
class SCOPY_CORE_EXPORT TriggerMarker : public QObject
{
	Q_OBJECT
public:
	explicit TriggerMarker(TriggerProcessor *proc, QObject *parent = nullptr);
	~TriggerMarker() override;

	// Put the marker on `axis` of `plot`, rebuilding the handle only if the plot changed.
	// Idempotent, so a caller may re-resolve on every event that could move the answer. A
	// null plot or axis is detach(). `axis` should be horizontal; callers resolve that.
	void attach(scopy::PlotWidget *plot, scopy::PlotAxis *axis);

	// Drop the handle, leaving the axis and the plot untouched.
	void detach();
	bool isAttached() const;

	// The window the sample↔axis map is measured over: `samples` slots spanning `xFirst` to
	// `xLast` in axis units. Explicit rather than read off the axis's current interval — a
	// zoom changes the interval without changing which X a sample is drawn at, so a map
	// derived from it skews by the zoom factor. `samples` <= 0 asks the processor instead.
	void setWindow(int samples, double xFirst, double xLast);

	// Re-apply visibility and position from the processor's state. Call after a change this
	// object cannot observe, such as the window width.
	void sync();

private:
	// Trigger on and not sample-specific: the bar means "position", not "target sample".
	bool scanActive() const;

	// The axis's live scale interval, from QwtPlot::canvasMap — the same map PlotAxisHandle
	// converts pixels with, so the two cannot disagree. Not PlotAxis::min()/max(): a zoom
	// goes straight to QwtPlot::setAxisScale and leaves those stale.
	bool range(double &lo, double &hi) const;

	int     samples() const;
	double  xForSample(quint32 sample) const;
	quint32 sampleForX(double x) const;
	// The last fire once there has been one, otherwise derived from the configured split.
	quint32 anchorSample() const;

	// Re-assert the scan-mode invariant: the anchored sample sits at m_frac of the canvas.
	// Shifts the interval and preserves its width, so a zoom performed meanwhile survives.
	//
	// Writes the PlotAxis directly, which is load-bearing in the acq plugin:
	// AcqAxis::requestInterval() drops a request identical to the last one it recorded, and
	// panning through it would overwrite that record — so the curve's unchanged per-frame
	// request would stop looking like a repeat and undo the pan once per frame.
	void align();

	// Repaint the canvas as well as the bar: the bar is an AxisHandle child of the canvas,
	// autoReplot is off, and the frame timer does not run while the acquisition is stopped.
	void repaintPlot();

	// axisScaleUpdated on the handle's axis. m_aligning breaks the recursion both for
	// align()'s own write and for PlotNavigator re-basing its zoomer off the same signal.
	void bindScale();

	QPointer<TriggerProcessor> m_proc;

	QPointer<scopy::PlotWidget>	m_plot;
	QPointer<scopy::PlotAxis>	m_axis;
	QPointer<scopy::PlotAxisHandle> m_handle;
	// The axis's axisScaleUpdated, re-made on every retarget.
	QMetaObject::Connection m_scaleConn;

	// Where the bar sits as a fraction of the canvas, and the source of truth for its place
	// in scan mode: a scale coordinate would move under the reader on every pan.
	double m_frac{0.5};

	// The sample pinned under the bar. Set by a fire, latched from the pre-drag fraction on
	// the first drag — pinned rather than recomputed per step, or align() is the identity.
	quint32 m_anchorSample{0};
	bool	m_anchorSet{false};

	// Recursion guard for align() → setInterval → axisScaleUpdated → align().
	bool m_aligning{false};

	// m_samples <= 0 means "ask the processor".
	int    m_samples{0};
	double m_x0{0.0};
	double m_x1{0.0};
};

} // namespace acq
} // namespace scopy

#endif // TRIGGERMARKER_H
