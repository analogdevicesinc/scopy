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

#include "triggermarker.h"

#include "triggerprocessor.h"

#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotwidget.h>

#include <QwtPlot>
#include <qwt_scale_map.h>

#include <algorithm>
#include <cmath>

using namespace scopy;
using namespace scopy::acq;

TriggerMarker::TriggerMarker(TriggerProcessor *proc, QObject *parent)
	: QObject(parent)
	, m_proc(proc)
{
	if(m_proc.isNull()) {
		return;
	}

	// The processor, not its widget: the split is also settable from the spinbox and a fire
	// moves the target on its own, so following the widget would miss both.
	connect(m_proc.data(), &TriggerProcessor::targetSampleChanged, this, [this](quint32) { sync(); });
	connect(m_proc.data(), &TriggerProcessor::sampleSpecificChanged, this, [this](bool) {
		// The anchor belongs to the mode that established it: carrying it across would pan
		// the axis to align a sample that no longer means anything.
		m_anchorSet = false;
		sync();
	});
	connect(m_proc.data(), &ProcessorBlock::enabledChanged, this, [this](bool) { sync(); });

	// Queued: fired() comes off the acquisition worker thread and everything below touches
	// widgets.
	connect(
		m_proc.data(), &TriggerProcessor::fired, this,
		[this](quint32 atSample, QMap<QString, SampleVariant>) {
			m_anchorSample = atSample;
			m_anchorSet = true;
			// In scan mode the axis is panned to put that sample under the bar, not the
			// bar moved to the sample: the bar is where the reader put it.
			if(scanActive()) {
				align();
				repaintPlot();
				return;
			}
			sync();
		},
		Qt::QueuedConnection);
}

TriggerMarker::~TriggerMarker() { detach(); }

bool TriggerMarker::isAttached() const { return !m_handle.isNull(); }

bool TriggerMarker::scanActive() const { return !m_proc.isNull() && m_proc->isEnabled() && !m_proc->sampleSpecific(); }

void TriggerMarker::setWindow(int samples, double xFirst, double xLast)
{
	if(samples == m_samples && qFuzzyCompare(1.0 + xFirst, 1.0 + m_x0) && qFuzzyCompare(1.0 + xLast, 1.0 + m_x1)) {
		return;
	}
	m_samples = samples;
	m_x0 = xFirst;
	m_x1 = xLast;
	sync();
}

int TriggerMarker::samples() const
{
	if(m_samples > 0) {
		return m_samples;
	}
	// The processor's own window: its fire index is expressed in these units.
	const int n = m_proc.isNull() ? 0 : m_proc->windowSize();
	return n > 0 ? n : 1;
}

bool TriggerMarker::range(double &lo, double &hi) const
{
	if(m_handle.isNull() || m_axis.isNull() || m_plot.isNull() || !m_plot->plot()) {
		return false;
	}
	// canvasMap rather than PlotAxis::min()/max(): see the header.
	const QwtScaleMap map = m_plot->plot()->canvasMap(m_axis->axisId());
	lo = map.s1();
	hi = map.s2();
	if(hi < lo) {
		std::swap(lo, hi);
	}
	return hi > lo;
}

double TriggerMarker::xForSample(quint32 sample) const
{
	const int last = std::max(0, samples() - 1);
	if(last == 0) {
		return m_x0;
	}
	// The window as the caller declared it, not the axis interval.
	const double span = (m_samples > 0) ? (m_x1 - m_x0) : static_cast<double>(last);
	const double base = (m_samples > 0) ? m_x0 : 0.0;
	const double frac = std::clamp(static_cast<double>(sample) / last, 0.0, 1.0);
	return base + frac * span;
}

quint32 TriggerMarker::sampleForX(double x) const
{
	const int last = std::max(0, samples() - 1);
	if(last == 0) {
		return 0;
	}
	const double span = (m_samples > 0) ? (m_x1 - m_x0) : static_cast<double>(last);
	const double base = (m_samples > 0) ? m_x0 : 0.0;
	if(!(std::abs(span) > 0.0)) {
		return 0;
	}
	const double frac = (x - base) / span;
	// Clamped, not trusted: the canvas edge maps a hair past the window and a zoomed axis
	// shows only part of it, so a fraction outside 0..1 is normal here.
	const int s = std::clamp(static_cast<int>(std::lround(frac * last)), 0, last);
	return static_cast<quint32>(s);
}

quint32 TriggerMarker::anchorSample() const
{
	if(m_anchorSet) {
		return m_anchorSample;
	}
	// No fire yet: the configured split says where one *will* land, so the bar and the axis
	// agree before the first fire. TriggerProcessor::process derives `pre` the same way.
	const int last = std::max(0, samples() - 1);
	const double frac = m_proc.isNull() ? 0.5 : std::clamp(m_proc->triggerPosition(), 0.0, 1.0);
	return static_cast<quint32>(std::clamp(static_cast<int>(std::lround(frac * last)), 0, last));
}

void TriggerMarker::align()
{
	if(m_aligning || !scanActive() || m_handle.isNull() || m_axis.isNull()) {
		return;
	}

	double lo = 0.0, hi = 0.0;
	if(!range(lo, hi)) {
		return;
	}
	const double width = hi - lo;

	const double anchorX = xForSample(anchorSample());
	const double frac = std::clamp(m_frac, 0.0, 1.0);
	const double newLo = anchorX - frac * width;

	m_aligning = true;
	// Width preserved, so this is a pan and the reader's zoom survives.
	m_axis->setInterval(newLo, newLo + width);
	// After the interval: the bar's scale position is read against the new one.
	m_handle->setPositionSilent(anchorX);
	m_aligning = false;
}

void TriggerMarker::repaintPlot()
{
	if(m_plot.isNull()) {
		return;
	}
	// Both widgets: see the header.
	m_plot->replot();
	if(!m_handle.isNull() && m_handle->handle()) {
		m_handle->handle()->repaint();
	}
}

void TriggerMarker::bindScale()
{
	if(m_scaleConn) {
		disconnect(m_scaleConn);
		m_scaleConn = {};
	}
	if(m_axis.isNull()) {
		return;
	}
	m_scaleConn = connect(m_axis.data(), &PlotAxis::axisScaleUpdated, this, [this]() { align(); });
}

void TriggerMarker::attach(PlotWidget *plot, PlotAxis *axis)
{
	if(!plot || !axis) {
		detach();
		return;
	}

	if(!m_handle.isNull() && m_plot == plot) {
		// Same canvas, so only the axis moves. Callers must keep the axis *position* the
		// same across this: PlotWidget's handle registry is keyed on it, so a setAxis
		// across positions would strand the old entry.
		if(m_axis != axis) {
			m_axis = axis;
			m_handle->setAxis(axis);
			bindScale();
		}
		sync();
		return;
	}

	// A different plot means a different canvas, and PlotAxisHandle takes its PlotWidget in
	// the constructor.
	detach();

	m_plot = plot;
	m_axis = axis;

	m_handle = new PlotAxisHandle(plot, axis);
	// ALWAYS, unlike the channel handles' ON_HOVER: the bar is what the reader aims at the
	// feature to trigger on, and one visible only under the cursor could not be aimed.
	m_handle->handle()->setBarVisibility(BarVisibility::ALWAYS);
	// Below the canvas, clear of the channel handles down the left edge.
	m_handle->handle()->setHandlePos(HandlePos::SOUTH_OR_EAST);
	plot->addPlotAxisHandle(m_handle);

	connect(m_handle.data(), &PlotAxisHandle::scalePosChanged, this, [this](double pos) {
		if(m_proc.isNull() || m_handle.isNull()) {
			return;
		}

		if(m_proc->sampleSpecific()) {
			// The axis stays put: the reader is pointing at data.
			m_proc->setTargetSample(sampleForX(pos));
			// Unconditional, not left to targetSampleChanged: one sample spans many
			// pixels, so most drag steps emit nothing and the drag looked choppy.
			repaintPlot();
			return;
		}

		double lo = 0.0, hi = 0.0;
		if(!range(lo, hi)) {
			return;
		}

		// Latch the anchor from where the bar *was* before this drag. Without it a drag
		// before the first fire aligns to the fraction it is being dragged to — the
		// identity — and the axis never moves.
		if(!m_anchorSet) {
			m_anchorSample = anchorSample();
			m_anchorSet = true;
		}

		m_frac = std::clamp((pos - lo) / (hi - lo), 0.0, 1.0);
		// The split the *next* fire uses, so where the bar sits and where the trigger
		// lands are one setting rather than two that can disagree.
		m_proc->setTriggerPosition(m_frac);
		align();
		repaintPlot();
	});

	bindScale();
	sync();
}

void TriggerMarker::detach()
{
	if(m_scaleConn) {
		disconnect(m_scaleConn);
		m_scaleConn = {};
	}

	if(!m_handle.isNull()) {
		PlotAxisHandle *h = m_handle.data();
		m_handle = nullptr;
		if(!m_plot.isNull()) {
			m_plot->removePlotAxisHandle(h);
		}
		// deleteLater: the handle filters the canvas's events and may be mid-dispatch.
		h->deleteLater();
	}

	// The axis is untouched on purpose: shared with every channel drawing that source, and
	// freed by the plot with the rest of its axis pool.
	m_plot = nullptr;
	m_axis = nullptr;
}

void TriggerMarker::sync()
{
	if(m_handle.isNull() || m_proc.isNull()) {
		return;
	}

	// Both widgets: the AxisHandle parents itself to the canvas rather than to its wrapper,
	// so hiding one does not hide the other.
	const bool vis = m_proc->isEnabled() && !m_axis.isNull();
	m_handle->setVisible(vis);
	if(m_handle->handle()) {
		m_handle->handle()->setVisible(vis);
	}

	if(m_proc->sampleSpecific()) {
		// Silent, or this re-enters setTargetSample from the signal reporting it.
		m_handle->setPositionSilent(xForSample(m_proc->targetSample()));
	} else {
		// m_frac is the source of truth in this mode: the bar goes back to it and the
		// axis is panned to suit, not the other way round.
		double lo = 0.0, hi = 0.0;
		if(range(lo, hi)) {
			m_handle->setPositionSilent(lo + std::clamp(m_frac, 0.0, 1.0) * (hi - lo));
		}
		align();
	}

	repaintPlot();
}

#include "moc_triggermarker.cpp"
