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

#include "plotscales.h"
#include "plotaxis.h"
#include "qpen.h"

#include <QwtPlotGrid>
#include <QwtPlotScaleItem>
#include <QwtScaleDiv>
#include <qwt_scale_widget.h>
#include <stylehelper.h>

#include <pluginbase/preferences.h>

using namespace scopy;

PlotScales::PlotScales(PlotWidget *plot)
	: QObject(plot)
	, m_plot(plot)
	, m_color(StyleHelper::GetInstance()->getColor("UIElementHighlight"))
{
	initGrid();
	initGraticule();
	initMarginScales();
}

PlotScales::~PlotScales() {}

void PlotScales::setGridEn(bool en)
{
	m_gridEn = en;
	if(en) {
		m_grid->attach(m_plot->plot());
	} else {
		m_grid->detach();
	}
	m_plot->replot();
}

bool PlotScales::getGridEn() { return m_gridEn; }

void PlotScales::initMarginScales()
{
	// margin scales are disabled only if graticule are enabled

	m_marginScales = new QList<QwtPlotScaleItem *>();
	for(QwtScaleDraw::Alignment scale :
	    {QwtScaleDraw::BottomScale, QwtScaleDraw::TopScale, QwtScaleDraw::LeftScale, QwtScaleDraw::RightScale}) {
		auto scaleItem = new EdgelessPlotScaleItem(scale);

		scaleItem->scaleDraw()->setAlignment(scale);
		scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
		scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
		scaleItem->setFont(m_plot->plot()->axisWidget(0)->font());

		QPalette palette = scaleItem->palette();
		palette.setBrush(QPalette::WindowText, m_color);
		palette.setBrush(QPalette::Text, m_color);
		scaleItem->setPalette(palette);
		scaleItem->setBorderDistance(0);
		m_marginScales->push_back(scaleItem);
		scaleItem->setZ(200);
	}

	setMarginScalesEn(!Preferences::GetInstance()->get("show_graticule").toBool());
	connect(Preferences::GetInstance(), &Preferences::preferenceChanged, this,
		[=](QString preference, QVariant value) {
			if(preference == "show_graticule") {
				setMarginScalesEn(!value.toBool());
			}
		});
}

void PlotScales::setMarginScalesEn(bool en)
{
	m_marginScalesEn = en;
	for(QwtPlotScaleItem *scale : *m_marginScales) {
		if(en) {
			scale->attach(m_plot->plot());
		} else {
			scale->detach();
		}
	}
}

bool PlotScales::getMarginScalesEn() { return m_marginScalesEn; }

void PlotScales::setGraticuleEn(bool en)
{
	m_graticuleEn = en;
	if(en) {
		m_x1Graticule->attach(m_plot->plot());
		m_x2Graticule->attach(m_plot->plot());

		m_y1Graticule->attach(m_plot->plot());
		m_y2Graticule->attach(m_plot->plot());
	} else {
		m_x1Graticule->detach();
		m_x2Graticule->detach();

		m_y1Graticule->detach();
		m_y2Graticule->detach();
	}
	m_plot->replot();
}

bool PlotScales::getGraticuleEn() { return m_graticuleEn; }

void PlotScales::initGrid()
{
	QPen gridpen(m_color);
	m_grid = new EdgelessPlotGrid();
	QColor majorPenColor(gridpen.color());
	m_grid->setMajorPen(majorPenColor, 1.0, Qt::DashLine);

	setGridEn(Preferences::GetInstance()->get("show_grid").toBool());
	connect(Preferences::GetInstance(), &Preferences::preferenceChanged, this,
		[=](QString preference, QVariant value) {
			if(preference == "show_grid") {
				setGridEn(value.toBool());
			}
		});
}

void PlotScales::initGraticule()
{
	QPalette palette = QPalette();
	palette.setBrush(QPalette::WindowText, m_color);
	palette.setBrush(QPalette::Text, Qt::transparent);

	m_x1Graticule = new QwtPlotScaleItem(QwtScaleDraw::BottomScale, 0.0);
	m_x1Graticule->setPalette(palette);
	m_x2Graticule = new QwtPlotScaleItem(QwtScaleDraw::TopScale, 0.0);
	m_x2Graticule->setPalette(palette);

	m_y1Graticule = new QwtPlotScaleItem(QwtScaleDraw::LeftScale, 0.0);
	m_y1Graticule->setPalette(palette);
	m_y2Graticule = new QwtPlotScaleItem(QwtScaleDraw::RightScale, 0.0);
	m_y2Graticule->setPalette(palette);

	setGraticuleEn(Preferences::GetInstance()->get("show_graticule").toBool());
	connect(Preferences::GetInstance(), &Preferences::preferenceChanged, this,
		[=](QString preference, QVariant value) {
			if(preference == "show_graticule") {
				setGraticuleEn(value.toBool());
			}
		});
	connect(m_plot, &PlotWidget::channelSelected, this, [=](PlotChannel *ch) {
		QwtAxisId xAxisId = m_plot->xAxis()->axisId();
		QwtAxisId yAxisId = m_plot->yAxis()->axisId();

		if(ch != nullptr) {
			xAxisId = ch->xAxis()->axisId();
			yAxisId = ch->yAxis()->axisId();
		}

		m_x1Graticule->setAxes(xAxisId, yAxisId);
		m_x2Graticule->setAxes(xAxisId, yAxisId);

		m_y1Graticule->setAxes(xAxisId, yAxisId);
		m_y2Graticule->setAxes(xAxisId, yAxisId);
		m_plot->replot();
	});
}

#include "moc_plotscales.cpp"
