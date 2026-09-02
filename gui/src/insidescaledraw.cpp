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

#include "insidescaledraw.h"

#include <QPainter>

using namespace scopy;

InsideScaleDraw::InsideScaleDraw(PrefixFormatter *formatter, const QString &unit)
	: BasicScaleDraw(formatter, unit)
{}

void InsideScaleDraw::drawLabel(QPainter *painter, double value) const
{
	const QRect bounds = boundingLabelRect(painter->font(), value);
	if(bounds.isEmpty()) {
		return;
	}

	// pos() is the scale's origin and length() its extent, both set by QwtPlotScaleItem::draw()
	// from canvasRect, so together they are the canvas bounds along the scale direction.
	double shift = 0.0;
	if(orientation() == Qt::Horizontal) {
		const double left = pos().x();
		const double right = left + length();
		if(bounds.left() < left) {
			shift = left - bounds.left();
		} else if(bounds.right() > right) {
			shift = right - bounds.right();
		}
	} else {
		const double top = pos().y();
		const double bottom = top + length();
		if(bounds.top() < top) {
			shift = top - bounds.top();
		} else if(bounds.bottom() > bottom) {
			shift = bottom - bounds.bottom();
		}
	}

	if(qFuzzyIsNull(shift)) {
		BasicScaleDraw::drawLabel(painter, value);
		return;
	}

	painter->save();
	if(orientation() == Qt::Horizontal) {
		painter->translate(shift, 0);
	} else {
		painter->translate(0, shift);
	}
	BasicScaleDraw::drawLabel(painter, value);
	painter->restore();
}
