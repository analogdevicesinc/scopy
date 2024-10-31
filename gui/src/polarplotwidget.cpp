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

#include "polarplotwidget.h"

#include <QGridLayout>

using namespace scopy;

PolarPlotWidget::PolarPlotWidget(QWidget *parent)
	: QWidget(parent)
	, m_plot(new QwtPolarPlot(this))
	, m_grid(new QwtPolarGrid())
{
	m_layout = new QGridLayout(this);
	m_layout->setSpacing(0);
	m_layout->setMargin(0);
	setLayout(m_layout);

	setAzimuthInterval(0.0, 360.0, 45.0);
	setRadiusInterval(0.0, 1.0, 0.2);
	setGridPen(QPen(Qt::white));
	showMinorGrid();
	setMinorGridPen(MinorGridStyle::MGS_DOT);
	showRadiusAxis(false, true, true, false);
	showAzimuthAxis();
	m_grid->attach(m_plot);
	m_layout->addWidget(m_plot);
}

PolarPlotWidget::~PolarPlotWidget() {}

void PolarPlotWidget::addPlotChannel(PolarPlotChannel *ch) { m_plotChannels.append(ch); }

void PolarPlotWidget::removePlotChannel(PolarPlotChannel *ch) { m_plotChannels.removeAll(ch); }

QwtPolarPlot *PolarPlotWidget::plot() const { return m_plot; }

void PolarPlotWidget::setAzimuthInterval(double min, double max, double step)
{
	m_plot->setScale(QwtPolar::Azimuth, min, max, step);
	m_plot->replot();
}

void PolarPlotWidget::setRadiusInterval(double min, double max, double step)
{
	m_plot->setScale(QwtPolar::Radius, min, max, step);
	m_plot->replot();
}

void PolarPlotWidget::showRadiusAxis(bool left, bool right, bool top, bool bottom)
{
	m_grid->showAxis(QwtPolar::AxisLeft, left);
	m_grid->showAxis(QwtPolar::AxisRight, right);
	m_grid->showAxis(QwtPolar::AxisTop, top);
	m_grid->showAxis(QwtPolar::AxisBottom, bottom);
}

void PolarPlotWidget::showAzimuthAxis(bool show) { m_grid->showAxis(QwtPolar::AxisAzimuth, show); }

void PolarPlotWidget::setGridPen(QPen pen) { m_grid->setPen(pen); }

void PolarPlotWidget::showMinorGrid(bool show)
{
	for(unsigned int i = 0; i < QwtPolar::ScaleCount; i++) {
		m_grid->showGrid(i, show);
		m_grid->showMinorGrid(i, show);
	}
}

void PolarPlotWidget::setMinorGridPen(int style, QPen pen)
{
	Qt::PenStyle penStyle;
	switch(style) {
	case MGS_LINES:
		penStyle = Qt::SolidLine;
		break;
	case MGS_DOT:
		penStyle = Qt::DotLine;
		break;
	case MGS_DASH:
		penStyle = Qt::DashLine;
		break;
	case MGS_DASH_DOT:
		penStyle = Qt::DashDotLine;
		break;
	case MGS_DASH_DOT_DOT:
		penStyle = Qt::DashDotDotLine;
		break;
	default:
		penStyle = Qt::DotLine;
		break;
	}
	for(unsigned int i = 0; i < QwtPolar::ScaleCount; i++) {
		QPen minorPen(pen);
		minorPen.setStyle(penStyle);
		m_grid->setMinorGridPen(i, minorPen);
	}
}

void PolarPlotWidget::setBgColor(const QColor &color) { m_plot->setPlotBackground(QBrush(color)); }

void PolarPlotWidget::setData(QVector<QVector<QwtPointPolar>> data)
{
	if(data.size() == m_plotChannels.size()) {
		int chnlIdx = 0;
		for(PolarPlotChannel *chnl : qAsConst(m_plotChannels)) {
			chnl->setSamples(data[chnlIdx]);
			chnlIdx++;
		}
	}
	m_plot->replot();
}

void PolarPlotWidget::replot() { m_plot->replot(); }

#include "moc_polarplotwidget.cpp"
