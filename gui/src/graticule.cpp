/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <scopy/gui/graticule.hpp>

using namespace scopy::gui;

Graticule::Graticule(QwtPlot* plot)
	: m_enabled(false)
	, m_plot(plot)
{
	m_vertScale = new GraticulePlotScaleItem(QwtScaleDraw::LeftScale, -1);
	m_horizScale = new GraticulePlotScaleItem(QwtScaleDraw::BottomScale, -1);
	m_vertScale2 = new GraticulePlotScaleItem(QwtScaleDraw::RightScale, -1);
	m_horizScale2 = new GraticulePlotScaleItem(QwtScaleDraw::TopScale, -1);

	QPalette palette = m_vertScale->palette();
	palette.setBrush(QPalette::Foreground, QColor("#6E6E6F"));
	palette.setBrush(QPalette::Text, QColor("#6E6E6F"));

	double minTick = m_vertScale->scaleDraw()->tickLength(QwtScaleDiv::MinorTick);
	double medTick = m_vertScale->scaleDraw()->tickLength(QwtScaleDiv::MediumTick);
	double majTick = m_vertScale->scaleDraw()->tickLength(QwtScaleDiv::MajorTick);

	m_vertScale->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	m_vertScale->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
	m_vertScale->scaleDraw()->setTickLength(QwtScaleDiv::MinorTick, minTick * 3 / 4);
	m_vertScale->scaleDraw()->setTickLength(QwtScaleDiv::MediumTick, medTick * 3 / 4);
	m_vertScale->scaleDraw()->setTickLength(QwtScaleDiv::MajorTick, majTick * 3 / 4);
	m_vertScale->setPalette(palette);

	m_horizScale->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	m_horizScale->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
	m_horizScale->scaleDraw()->setTickLength(QwtScaleDiv::MinorTick, minTick * 3 / 4);
	m_horizScale->scaleDraw()->setTickLength(QwtScaleDiv::MediumTick, medTick * 3 / 4);
	m_horizScale->scaleDraw()->setTickLength(QwtScaleDiv::MajorTick, majTick * 3 / 4);
	m_horizScale->setPalette(palette);

	m_vertScale2->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	m_vertScale2->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
	m_vertScale2->scaleDraw()->setTickLength(QwtScaleDiv::MinorTick, minTick * 3 / 4);
	m_vertScale2->scaleDraw()->setTickLength(QwtScaleDiv::MediumTick, medTick * 3 / 4);
	m_vertScale2->scaleDraw()->setTickLength(QwtScaleDiv::MajorTick, majTick * 3 / 4);
	m_vertScale2->setPalette(palette);

	m_horizScale2->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	m_horizScale2->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
	m_horizScale2->scaleDraw()->setTickLength(QwtScaleDiv::MinorTick, minTick * 3 / 4);
	m_horizScale2->scaleDraw()->setTickLength(QwtScaleDiv::MediumTick, medTick * 3 / 4);
	m_horizScale2->scaleDraw()->setTickLength(QwtScaleDiv::MajorTick, majTick * 3 / 4);
	m_horizScale2->setPalette(palette);
}

Graticule::~Graticule()
{
	delete m_vertScale;
	delete m_vertScale2;
	delete m_horizScale;
	delete m_horizScale2;
}

void Graticule::toggleGraticule()
{
	m_enabled = !m_enabled;

	if (m_enabled) {
		m_vertScale->attach(m_plot);
		m_vertScale2->attach(m_plot);
		m_horizScale->attach(m_plot);
		m_horizScale2->attach(m_plot);
	} else {
		m_vertScale->detach();
		m_vertScale2->detach();
		m_horizScale->detach();
		m_horizScale2->detach();
	}
}

void Graticule::enableGraticule(bool enable)
{
	if (enable != m_enabled) {
		m_enabled = enable;

		if (m_enabled) {
			m_vertScale->attach(m_plot);
			m_vertScale2->attach(m_plot);
			m_horizScale->attach(m_plot);
			m_horizScale2->attach(m_plot);
		} else {
			m_vertScale->detach();
			m_vertScale2->detach();
			m_horizScale->detach();
			m_horizScale2->detach();
		}
	}
}

void Graticule::onCanvasSizeChanged()
{
	m_vertScale->setBorderDistance(m_plot->canvas()->width() / 2);
	m_horizScale->setBorderDistance(m_plot->canvas()->height() / 2);
	m_vertScale2->setBorderDistance(m_plot->canvas()->width() / 2);
	m_horizScale2->setBorderDistance(m_plot->canvas()->height() / 2);
}

GraticulePlotScaleItem::GraticulePlotScaleItem(QwtScaleDraw::Alignment alignment, const double pos)
	: QwtPlotScaleItem(alignment, pos)
{}

static QwtScaleDiv getGraticuleScaleDiv(const QwtScaleDiv& from_scaleDiv)
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

	if (majorTicks.size() >= 2) {
		majorTicks.erase(majorTicks.begin());
		majorTicks.erase(majorTicks.end() - 1);
	}

	return QwtScaleDiv(lowerBound, upperBound, minorTicks, mediumTicks, majorTicks);
}

void GraticulePlotScaleItem::updateScaleDiv(const QwtScaleDiv& xScaleDiv, const QwtScaleDiv& yScaleDiv)
{
	QwtPlotScaleItem::updateScaleDiv(getGraticuleScaleDiv(xScaleDiv), getGraticuleScaleDiv(yScaleDiv));
}
