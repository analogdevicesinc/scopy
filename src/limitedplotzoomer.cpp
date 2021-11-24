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
#include "limitedplotzoomer.h"
#include <qwt_plot.h>
#include <QtDebug>

using namespace adiscope;

LimitedPlotZoomer::LimitedPlotZoomer(QWidget *parent, bool doReplot):
	QwtPlotZoomer(parent, doReplot),
	m_boundVertical(false), m_updateBaseNextZoom(true)
{
        setMaxStackDepth(5);
}

void LimitedPlotZoomer::resetZoom()
{
	QwtPlotZoomer::zoom(0);
	m_updateBaseNextZoom = true;
}

void LimitedPlotZoomer::popZoom()
{
	QwtPlotZoomer::zoom(-1);
}

void LimitedPlotZoomer::setBoundVertical(bool bound)
{
	m_boundVertical = bound;
}

void LimitedPlotZoomer::zoom(const QRectF &rect)
{

	if(m_updateBaseNextZoom) {
		setZoomBase();
		m_updateBaseNextZoom = false;
	}
	QRectF boundedRect = rect & zoomBase();

	if (m_boundVertical) {
		QRectF baseRect = zoomBase();
		boundedRect.setTop(baseRect.top());
		boundedRect.setBottom(baseRect.bottom());
	}

	QwtPlotZoomer::zoom(boundedRect);
}



void LimitedPlotZoomer::axesChanged() {
	qDebug()<<"Axes changed";
}
QSizeF LimitedPlotZoomer::minZoomSize() const
{
	const double eps = 10e12;

	return {zoomBase().width() / eps, zoomBase().height() / eps};
}
