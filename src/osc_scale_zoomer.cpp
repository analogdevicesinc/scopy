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
	font.setWeight(QFont::Bold);

	setTrackerFont(font);
	setZoomBase();
}

OscScaleZoomer::~OscScaleZoomer()
{
}

QwtText OscScaleZoomer::trackerText(const QPoint& pos) const
{
	const OscScaleDraw *draw_x = static_cast<const OscScaleDraw *>(
	                                     plot()->axisScaleDraw(QwtAxis::XTop));
	const OscScaleDraw *draw_y = static_cast<const OscScaleDraw *>(
	                                     plot()->axisScaleDraw(QwtAxis::YLeft));
	QPointF dp = QwtPlotZoomer::invTransform(pos);
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
