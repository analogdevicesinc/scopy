/*
 * Copyright 2017 Analog Devices, Inc.
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

#include "DisplayPlot.h"
#include "osc_scale_zoomer.h"

#include <QString>

using namespace adiscope;

OscScaleZoomer::OscScaleZoomer(QWidget *parent) :
	LimitedPlotZoomer(parent)
{
	setTrackerMode(QwtPicker::AlwaysOn);

	QPen pen(QColor("#999999"));
	setRubberBandPen(pen);
	setTrackerPen(pen);

	QFont font;
	font.setPointSize(10);
	font.setWeight(75);

	setTrackerFont(font);
	setZoomBase();
}

OscScaleZoomer::~OscScaleZoomer()
{
}

QwtText OscScaleZoomer::trackerText(const QPoint& pos) const
{
	const OscScaleDraw *draw_x = static_cast<const OscScaleDraw *>(
	                                     plot()->axisScaleDraw(QwtPlot::xTop));
	const OscScaleDraw *draw_y = static_cast<const OscScaleDraw *>(
	                                     plot()->axisScaleDraw(QwtPlot::yLeft));
	QwtDoublePoint dp = QwtPlotZoomer::invTransform(pos);
	QString text;

	text += draw_x->label(dp.x()).text();
	text += ", ";
	text += draw_y->label(dp.y()).text();

	return QwtText(text);
}

void OscScaleZoomer::cancel()
{
	reset();
}

QColor OscScaleZoomer::getColor() const
{
	return trackerPen().color();
}

void OscScaleZoomer::setColor(const QColor& color)
{
	QPen pen(color);
	setRubberBandPen(pen);
	setTrackerPen(pen);
}
