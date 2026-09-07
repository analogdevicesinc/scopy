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
#include <qwt_scale_div.h>

using namespace scopy;

InsideScaleDraw::InsideScaleDraw(PrefixFormatter *formatter, const QString &unit)
	: BasicScaleDraw(formatter, unit)
	, m_startMargin(0.0)
	, m_endMargin(0.0)
{}

void InsideScaleDraw::setEndMargins(double start, double end)
{
	m_startMargin = start;
	m_endMargin = end;
}

double InsideScaleDraw::startMargin() const { return m_startMargin; }

double InsideScaleDraw::endMargin() const { return m_endMargin; }

// Fills `bounds` with the label's unshifted rect and `shift` with how far along the scale it
// has to move to stay inside the window. False means the label cannot be drawn at all: it is
// empty, or it is longer than the window and could only ever be shown overhanging.
bool InsideScaleDraw::labelShift(const QFont &font, double value, QRect &bounds, double &shift) const
{
	bounds = boundingLabelRect(font, value);
	shift = 0.0;
	if(bounds.isEmpty()) {
		return false;
	}

	// pos() is the scale's origin and length() its extent, both set by QwtPlotScaleItem::draw()
	// from canvasRect, so together they are the canvas bounds along the scale direction.
	const bool horizontal = (orientation() == Qt::Horizontal);
	const double origin = horizontal ? pos().x() : pos().y();
	const double lowerEdge = horizontal ? bounds.left() : bounds.top();
	const double upperEdge = horizontal ? bounds.right() : bounds.bottom();

	const double windowStart = origin + m_startMargin;
	const double windowEnd = origin + length() - m_endMargin;

	// A label wider than the window can only be drawn overlapping, so drop it instead.
	if(upperEdge - lowerEdge > windowEnd - windowStart) {
		return false;
	}

	if(lowerEdge < windowStart) {
		shift = windowStart - lowerEdge;
	} else if(upperEdge > windowEnd) {
		shift = windowEnd - upperEdge;
	}
	return true;
}

// The base pass eliminates overlap on labels centred on their ticks, which is not where these
// end up: shifting the end labels inwards moves them *towards* their neighbours, so a pair the
// base class measured as clear can collide once drawn. Hence the second pass here, over the
// shifted rects.
//
// The two interval bounds are placed first and never dropped. They are the labels worth keeping —
// they say what the axis actually spans, and are the whole reason the ends get shifted — so when
// something has to give it is an interior label, which is one of many and reconstructible from its
// neighbours.
void InsideScaleDraw::draw(QPainter *painter, const QPalette &palette) const
{
	Q_UNUSED(palette)

	const QList<double> ticks = scaleDiv().ticks(QwtScaleDiv::MajorTick);
	if(ticks.isEmpty()) {
		return;
	}

	// label() derives its delta step from the tick count, and nothing else sets it once this
	// override takes over from BasicScaleDraw::draw().
	setNrTicks(static_cast<unsigned int>(ticks.size()));

	const bool horizontal = (orientation() == Qt::Horizontal);
	// The same padding the base pass uses, so two labels that read as touching are treated as
	// overlapping here too.
	const int half = painter->font().pointSize() / 4;

	QList<QRect> placed;
	auto place = [&](double tick) {
		QRect bounds;
		double shift = 0.0;
		if(!labelShift(painter->font(), tick, bounds, shift)) {
			return;
		}

		if(horizontal) {
			bounds.translate(qRound(shift), 0);
			bounds.adjust(-half, 0, half, 0);
		} else {
			bounds.translate(0, qRound(shift));
			bounds.adjust(0, -half / 2, 0, half / 2);
		}

		// Every kept rect, not just the previous one: the bounds are placed out of order, so
		// the neighbour to test against has not necessarily been seen yet.
		for(const QRect &other : std::as_const(placed)) {
			if(bounds.intersects(other)) {
				return;
			}
		}

		placed.append(bounds);
		drawLabel(painter, tick);
	};

	const int last = ticks.size() - 1;
	place(ticks.first());
	if(last > 0) {
		place(ticks.last());
	}

	// Dropped one at a time rather than by halving the tick list: a collision here is usually
	// between a shifted end label and its one neighbour, so thinning the whole scale would
	// throw away labels that had room.
	for(int i = 1; i < last; ++i) {
		place(ticks[i]);
	}
}

void InsideScaleDraw::drawLabel(QPainter *painter, double value) const
{
	QRect bounds;
	double shift = 0.0;
	if(!labelShift(painter->font(), value, bounds, shift)) {
		return;
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
