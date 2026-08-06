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

#include "acqcurverepr.h"

#include "acqplotrow.h"

#include <core/acq_engine/DataStore.h>

#include <gui/plotaxis.h>
#include <gui/plotchannel.h>
#include <gui/plotwidget.h>

#include <QPen>

using namespace scopy;
using namespace scopy::adc;

CurveRepr::CurveRepr() = default;

CurveRepr::~CurveRepr() { detach(); }

void CurveRepr::attach(AcqPlotRow *row, const QString &name, const QColor &color)
{
	if(!row || !row->plot() || m_ch) {
		return;
	}
	m_plot = row->plot();

	// The row's shared X and Y axes, never a per-channel one — see the axis
	// lifetime note in acqplotrow.h.
	m_ch = new PlotChannel(name, QPen(color), m_plot->xAxis(), m_plot->yAxis(), nullptr);

	// addPlotChannel() calls init(), which is what creates the QwtPlotCurve; the
	// PlotChannel constructor leaves it uninitialised. So nothing may call
	// setSamples() before this line.
	m_plot->addPlotChannel(m_ch);
	m_ch->setEnabled(m_enabled);
}

void CurveRepr::detach()
{
	// The plot may already be gone: it and the channel owning this repr are
	// siblings under the manager, so their destruction order is unspecified. Once
	// the QwtPlot is destroyed Qwt has auto-detached its items and touching them is
	// a use-after-free.
	if(m_plot.isNull()) {
		m_ch = nullptr;
		return;
	}
	if(!m_ch) {
		return;
	}

	PlotChannel *ch = m_ch;
	m_ch = nullptr; // before the call, so a re-entrant detach() is a no-op

	// Idempotence matters beyond tidiness: PlotTracker::removeChannel leaves its
	// `toRemove` pointer uninitialised when the channel is not in its list
	// (gui/src/plottracker.cpp:66-76), so a second removePlotChannel() for the same
	// channel is a wild delete.
	m_plot->removePlotChannel(ch);
	delete ch;

	// removePlotChannel() hides the removed channel's Y axis unconditionally
	// (gui/src/plotwidget.cpp:181). With a shared axis that hides it for every
	// remaining channel, and it is only restored if the removed channel happened to
	// be the selected one. Re-assert it.
	m_plot->showAxisLabels();
}

std::size_t CurveRepr::claimDepth(int plotSize, std::size_t bufferSize) const
{
	return scopy::acq::DataStore::depthForWindow(static_cast<std::size_t>(qMax(1, plotSize)), bufferSize);
}

void CurveRepr::pull(scopy::acq::DataStore *store, const scopy::acq::DataKey &key, int plotSize)
{
	if(!store || !m_ch || m_plot.isNull()) {
		return;
	}

	// Assign the member, then view it. Both m_live and m_scratch outlive the view,
	// and the view does not outlive this function.
	m_live = store->window(key, plotSize);
	const scopy::acq::FloatView y = scopy::acq::toFloatView(m_live, m_scratch);
	if(y.size <= 0) {
		return;
	}

	const float *xPtr = nullptr;
	int n = 0;

	if(!m_xKey.key.isEmpty()) {
		m_liveX = store->window(m_xKey, plotSize);
		const scopy::acq::FloatView x = scopy::acq::toFloatView(m_liveX, m_scratchX);
		if(x.size <= 0) {
			return;
		}
		n = qMin(x.size, y.size);
		xPtr = x.data;
	} else {
		if(!m_indexSrc || m_indexSrc->isEmpty()) {
			return;
		}
		// A partial window is shorter, not left-padded (SampleBuffer::window), so
		// right-anchor the shared ramp against it rather than starting at 0.
		const int off = qMax(0, m_indexSrc->size() - y.size);
		xPtr = m_indexSrc->constData() + off;
		n = qMin(y.size, m_indexSrc->size() - off);
	}

	if(n <= 0) {
		return;
	}

	// copy=true: Qwt keeps its own buffer, so no FloatView outlives this call and
	// nothing can paint a prior run's tail. Same choice and same reasoning as
	// src/sim/siminstrumentcontroller.cpp:1077-1081.
	m_ch->setSamples(xPtr, y.data, static_cast<size_t>(n), true);
}

void CurveRepr::reset()
{
	// Drop every cached window so no prior-run buffer is reachable on the next
	// cycle.
	m_live = scopy::acq::SampleVariant{};
	m_liveX = scopy::acq::SampleVariant{};
	m_scratch.clear();
	m_scratchX.clear();

	if(!m_ch || m_plot.isNull()) {
		return;
	}
	// Blank the curve rather than leave the previous run's tail on screen. A dummy
	// pointer with size 0, not nullptr: some Qwt paths dereference the data pointer
	// even for empty ranges (src/sim/siminstrumentcontroller.cpp:798-803).
	static const float kZero = 0.0f;
	m_ch->setSamples(&kZero, &kZero, 0, true);
}

void CurveRepr::setEnabled(bool en)
{
	m_enabled = en;
	if(!m_ch || m_plot.isNull()) {
		return;
	}
	if(en) {
		m_ch->enable();
	} else {
		m_ch->disable();
	}
}

void CurveRepr::setColor(const QColor &c)
{
	if(m_ch && !m_plot.isNull()) {
		m_ch->setColor(c);
	}
}

void CurveRepr::setName(const QString &n)
{
	if(m_ch && !m_plot.isNull()) {
		m_ch->setName(n);
	}
}
