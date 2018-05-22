/*
 * Copyright 2018 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "graticule.h"

using namespace adiscope;

Graticule::Graticule(QwtPlot *plot):
	enabled(false),
	plot(plot)
{
	vertScale = new GraticulePlotScaleItem(QwtScaleDraw::LeftScale,-1);
	horizScale = new GraticulePlotScaleItem(QwtScaleDraw::BottomScale,-1);
	vertScale2 = new GraticulePlotScaleItem(QwtScaleDraw::RightScale,-1);
	horizScale2 = new GraticulePlotScaleItem(QwtScaleDraw::TopScale,-1);

	QPalette palette = vertScale->palette();
	palette.setBrush(QPalette::Foreground, QColor("#6E6E6F"));
	palette.setBrush(QPalette::Text, QColor("#6E6E6F"));

	double minTick = vertScale->scaleDraw()->tickLength(QwtScaleDiv::MinorTick);
	double medTick = vertScale->scaleDraw()->tickLength(QwtScaleDiv::MediumTick);
	double majTick = vertScale->scaleDraw()->tickLength(QwtScaleDiv::MajorTick);

	vertScale->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	vertScale->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
	vertScale->scaleDraw()->setTickLength(QwtScaleDiv::MinorTick,minTick*3/4);
	vertScale->scaleDraw()->setTickLength(QwtScaleDiv::MediumTick,medTick*3/4);
	vertScale->scaleDraw()->setTickLength(QwtScaleDiv::MajorTick,majTick*3/4);
	vertScale->setPalette(palette);

	horizScale->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	horizScale->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
	horizScale->scaleDraw()->setTickLength(QwtScaleDiv::MinorTick,minTick*3/4);
	horizScale->scaleDraw()->setTickLength(QwtScaleDiv::MediumTick,medTick*3/4);
	horizScale->scaleDraw()->setTickLength(QwtScaleDiv::MajorTick,majTick*3/4);
	horizScale->setPalette(palette);

	vertScale2->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	vertScale2->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
	vertScale2->scaleDraw()->setTickLength(QwtScaleDiv::MinorTick,minTick*3/4);
	vertScale2->scaleDraw()->setTickLength(QwtScaleDiv::MediumTick,medTick*3/4);
	vertScale2->scaleDraw()->setTickLength(QwtScaleDiv::MajorTick,majTick*3/4);
	vertScale2->setPalette(palette);

	horizScale2->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	horizScale2->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
	horizScale2->scaleDraw()->setTickLength(QwtScaleDiv::MinorTick,minTick*3/4);
	horizScale2->scaleDraw()->setTickLength(QwtScaleDiv::MediumTick,medTick*3/4);
	horizScale2->scaleDraw()->setTickLength(QwtScaleDiv::MajorTick,majTick*3/4);
	horizScale2->setPalette(palette);
}

void Graticule::toggleGraticule(){
	enabled = !enabled;

	if(enabled){
		vertScale->attach(plot);
		vertScale2->attach(plot);
		horizScale->attach(plot);
		horizScale2->attach(plot);
	}
	else{
		vertScale->detach();
		vertScale2->detach();
		horizScale->detach();
		horizScale2->detach();
	}
}

void Graticule::enableGraticule(bool enable){
	if(enable != enabled){
		enabled = enable;

		if(enabled){
			vertScale->attach(plot);
			vertScale2->attach(plot);
			horizScale->attach(plot);
			horizScale2->attach(plot);
		}
		else{
			vertScale->detach();
			vertScale2->detach();
			horizScale->detach();
			horizScale2->detach();
		}
	}
}

void Graticule::onCanvasSizeChanged(){
	vertScale->setBorderDistance(plot->canvas()->width()/2);
	horizScale->setBorderDistance(plot->canvas()->height()/2);
	vertScale2->setBorderDistance(plot->canvas()->width()/2);
	horizScale2->setBorderDistance(plot->canvas()->height()/2);
}

GraticulePlotScaleItem::GraticulePlotScaleItem(
	QwtScaleDraw::Alignment alignment, const double pos ):
    QwtPlotScaleItem(alignment, pos)
{
}

void GraticulePlotScaleItem::updateScaleDiv( const QwtScaleDiv& xScaleDiv,
    const QwtScaleDiv& yScaleDiv )
{
	QwtPlotScaleItem::updateScaleDiv(getGraticuleScaleDiv(xScaleDiv),
					getGraticuleScaleDiv(yScaleDiv));
}

QwtScaleDiv adiscope::getGraticuleScaleDiv(const QwtScaleDiv& from_scaleDiv)
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



