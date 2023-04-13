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
#include "x_axis_scale_zoomer.h"
#include "qwt_scale_draw.h"
#include <qwt_text.h>

#include <dbgraph.hpp>

using namespace scopy;

XAxisScaleZoomer::XAxisScaleZoomer(QWidget *parent):
	OscScaleZoomer(parent)
{
}

XAxisScaleZoomer::~XAxisScaleZoomer()
{
}

void XAxisScaleZoomer::zoom(const QRectF& rect)
{
	QRectF boundedRect = rect & zoomBase();
	QRectF baseRect = zoomBase();

	boundedRect.setTop(baseRect.top());
	boundedRect.setBottom(baseRect.bottom());

	QwtPlotZoomer::zoom(boundedRect);
}

QwtText XAxisScaleZoomer::trackerText(const QPoint &p) const
{
	QwtText t;
	QPointF dp = QwtPlotZoomer::invTransform(p);

	const dBgraph *plt = dynamic_cast<const dBgraph *>(plot());
	t.setText(plt->getScaleValueFormat(dp.x(), QwtAxis::XTop, 4) + ", " +
		  plt->getScaleValueFormat(dp.y(), QwtAxis::YLeft));
	return t;
}
