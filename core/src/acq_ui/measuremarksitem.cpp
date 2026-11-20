/*
 * Copyright (c) 2026 Analog Devices Inc.
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
 */

#include "measuremarksitem.h"

#include <QPainter>
#include <QPen>

#include <qwt_scale_map.h>

using namespace scopy::acq;

MeasureMarksItem::MeasureMarksItem()
{
	setZ(100);
	setItemAttribute(QwtPlotItem::AutoScale, false);
	setItemAttribute(QwtPlotItem::Legend, false);
}

void MeasureMarksItem::setMarks(const QVector<double> &times, const QVector<double> &levels)
{
	m_times = times;
	m_levels = levels;
	itemChanged();
}

void MeasureMarksItem::clear() { setMarks({}, {}); }

void MeasureMarksItem::setColor(const QColor &c)
{
	m_color = c;
	itemChanged();
}

void MeasureMarksItem::draw(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap,
			    const QRectF &canvasRect) const
{
	painter->save();
	painter->setClipRect(canvasRect);
	QPen pen(m_color, 1, Qt::DashLine);
	pen.setCosmetic(true);
	painter->setPen(pen);
	for(double t : m_times) {
		const double x = xMap.transform(t);
		painter->drawLine(QPointF(x, canvasRect.top()), QPointF(x, canvasRect.bottom()));
	}
	for(double v : m_levels) {
		const double y = yMap.transform(v);
		painter->drawLine(QPointF(canvasRect.left(), y), QPointF(canvasRect.right(), y));
	}
	painter->restore();
}
