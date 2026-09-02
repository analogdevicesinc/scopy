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

#include "edgelessplot.h"

#include <QwtPlot>

using namespace scopy;

static QwtScaleDiv getEdgelessScaleDiv(const QwtScaleDiv &from_scaleDiv);

static QwtScaleDiv getEdgelessScaleDiv(const QwtScaleDiv &from_scaleDiv)
{
	double lowerBound;
	double upperBound;
	QList<double> minorTicks;
	QList<double> mediumTicks;
	QList<double> majorTicks;

	lowerBound = from_scaleDiv.lowerBound();
	upperBound = from_scaleDiv.upperBound();
	minorTicks = from_scaleDiv.ticks(QwtScaleDiv::MinorTick);
	mediumTicks = from_scaleDiv.ticks(QwtScaleDiv::MediumTick);
	majorTicks = from_scaleDiv.ticks(QwtScaleDiv::MajorTick);
	if(majorTicks.size() >= 2) {
		majorTicks.erase(majorTicks.begin());
		majorTicks.erase(majorTicks.end() - 1);
	}
	return QwtScaleDiv(lowerBound, upperBound, minorTicks, mediumTicks, majorTicks);
}
/*
 * EdgelessPlotScaleItem class implementation
 */
EdgelessPlotScaleItem::EdgelessPlotScaleItem(QwtScaleDraw::Alignment alignment, const double pos)
	: QwtPlotScaleItem(alignment, pos)
{}

void EdgelessPlotScaleItem::updateScaleDiv(const QwtScaleDiv &xScaleDiv, const QwtScaleDiv &yScaleDiv)
{
	if(!m_edgeless) {
		QwtPlotScaleItem::updateScaleDiv(xScaleDiv, yScaleDiv);
		return;
	}

	QwtPlotScaleItem::updateScaleDiv(getEdgelessScaleDiv(xScaleDiv), getEdgelessScaleDiv(yScaleDiv));
}

void EdgelessPlotScaleItem::setEdgeless(bool edgeless)
{
	if(m_edgeless == edgeless) {
		return;
	}

	m_edgeless = edgeless;

	// The trimmed scale div is already cached, so recompute it from the plot's current one
	// instead of waiting for the next scale change to bring the end ticks back.
	if(const QwtPlot *plt = plot()) {
		updateScaleDiv(plt->axisScaleDiv(xAxis()), plt->axisScaleDiv(yAxis()));
	}
}

bool EdgelessPlotScaleItem::isEdgeless() const { return m_edgeless; }

/*
 * EdgelessPlotGrid class implementation
 */
EdgelessPlotGrid::EdgelessPlotGrid()
	: QwtPlotGrid()
{}

void EdgelessPlotGrid::updateScaleDiv(const QwtScaleDiv &xScaleDiv, const QwtScaleDiv &yScaleDiv)
{
	QwtPlotGrid::updateScaleDiv(getEdgelessScaleDiv(xScaleDiv), getEdgelessScaleDiv(yScaleDiv));
}
