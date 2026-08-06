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

#include "acqplotrow.h"

#include <gui/plotaxis.h>
#include <gui/plotwidget.h>
#include <gui/style.h>
#include <gui/style_attributes.h>

#include <qwt_axis.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>

#include <QPen>

using namespace scopy;
using namespace scopy::adc;

namespace {

// Vertical stacking pitch in canvas pixels: 24 px band + 2 px gap. Kept in sync
// with DigitalCurveItem/AnnotationCurve visuals — same constant as
// src/sim/DigitalTrackManager.cpp:26-29.
constexpr double kSlotPitchPx = 26.0;

// Headroom above the first band, as a fraction of the axis span.
constexpr double kTopMargin = 0.10;

} // namespace

AcqPlotRow::AcqPlotRow(PlotWidget *plot, bool exclusive, QObject *parent)
	: QObject(parent)
	, m_plot(plot)
	, m_exclusive(exclusive)
{
}

AcqPlotRow::~AcqPlotRow()
{
	// m_digitalAxis is deliberately not deleted: PlotWidget has no
	// removePlotAxis() and PlotAxis's destructor does not unregister itself, so
	// deleting it would leave a dangling entry for PlotNavigator to walk. The axis
	// is parented to this row, so Qt frees the QObject — which is exactly as much
	// as is safe.
}

PlotWidget *AcqPlotRow::plot() const { return m_plot.data(); }

PlotAxis *AcqPlotRow::digitalAxis()
{
	if(m_digitalAxis) {
		return m_digitalAxis;
	}
	if(m_plot.isNull()) {
		return nullptr;
	}

	// A stacked extra YLeft axis, so handles render on the left of the canvas next
	// to the main analog Y axis. Fixed [0,1] with ticks and labels hidden: items
	// self-place using pixel offsets from their handle, so the axis exists only to
	// host them.
	QPen pen(Style::getColor(json::theme::content_silent));
	m_digitalAxis = new PlotAxis(QwtAxis::YLeft, m_plot.data(), pen, this);
	m_digitalAxis->setInterval(0.0, 1.0);
	m_digitalAxis->setVisible(false);
	return m_digitalAxis;
}

double AcqPlotRow::nextDigitalSlot()
{
	PlotAxis *ax = digitalAxis();
	if(!ax) {
		return 1.0;
	}

	if(!m_slotInit) {
		// Start below the top so the first band has headroom above it.
		const double span = ax->max() - ax->min();
		m_nextSlot = ax->max() - kTopMargin * span;
		m_slotInit = true;
	}

	const double pos = m_nextSlot;

	// Pitch pixels to scale delta. The axis is [0,1] across the canvas height, so
	// the conversion is exact once the canvas has a height; before first layout
	// there is nothing to measure and we fall back to a nominal 2% step, which
	// later handle drags can correct.
	double delta = 0.02;
	if(!m_plot.isNull() && m_plot->plot() && m_plot->plot()->canvas()) {
		const int h = m_plot->plot()->canvas()->height();
		if(h > 0) {
			delta = kSlotPitchPx / static_cast<double>(h);
		}
	}
	m_nextSlot = pos - delta;
	return pos;
}

void AcqPlotRow::replot()
{
	if(m_plot.isNull()) {
		return;
	}
	m_plot->replot();
}

#include "moc_acqplotrow.cpp"
