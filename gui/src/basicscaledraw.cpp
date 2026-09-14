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

#include "basicscaledraw.h"

#include <QPainter>
#include <QRect>
#include <qwt_interval.h>
#include <cmath>
#include <QwtText>

using namespace scopy;

BasicScaleDraw::BasicScaleDraw(const QString &unit)
	: QwtScaleDraw()
	, m_floatPrecision(3)
	, m_unit(unit)
	, m_formatter(NULL)
	, m_color(Qt::gray)
	, m_displayScale(1)
	, m_shouldDrawMiddleDelta(false)
	, m_nrTicks(0)
	, m_delta(false)
{
	enableComponent(QwtAbstractScaleDraw::Backbone, false);
	enableComponent(QwtAbstractScaleDraw::Ticks, false);
}

BasicScaleDraw::BasicScaleDraw(PrefixFormatter *formatter, const QString &unit)
	: BasicScaleDraw(unit)
{
	m_formatter = formatter;
}

void BasicScaleDraw::setFloatPrecision(unsigned int numDigits) { m_floatPrecision = numDigits; }

unsigned int BasicScaleDraw::getFloatPrecison() const { return m_floatPrecision; }

void BasicScaleDraw::setUnitType(const QString &unit)
{
	if(m_unit != unit) {
		m_unit = unit;

		// Trigger a new redraw of scale labels since there's a new unit that needs to be redrawn
		invalidateCache();
	}
}

QString BasicScaleDraw::getUnitType() const { return m_unit; }

void BasicScaleDraw::setColor(QColor color) { m_color = color; }

QColor BasicScaleDraw::getColor() { return m_color; }

void BasicScaleDraw::setDisplayScale(double value) { m_displayScale = value; }

void BasicScaleDraw::setFormatter(PrefixFormatter *formatter) { m_formatter = formatter; }

void BasicScaleDraw::enableDeltaLabel(bool enable)
{
	if(enable != m_delta) {
		m_delta = enable;

		// Trigger a new redraw of the scale
		invalidateCache();
	}
}

void BasicScaleDraw::setUnitsEnabled(bool enable) { m_unitsEn = enable; }

void BasicScaleDraw::draw(QPainter *painter, const QPalette &palette) const
{
	const QList<double> allTicks = scaleDiv().ticks(QwtScaleDiv::MajorTick);
	m_nrTicks = allTicks.size();

	if(allTicks.isEmpty())
		return;

	// pointSize() is -1 for pixel-sized fonts (Qt6/ARM) -> /4 would give 0 padding
	int half = painter->font().pointSize() / 4;
	if(half <= 0)
		half = painter->fontMetrics().height() / 4;

	QList<QRect> bounds;
	bounds.reserve(allTicks.size());
	for(int i = 0; i < allTicks.size(); ++i) {
		QRect r = boundingLabelRect(painter->font(), allTicks[i]);
		if(orientation() == Qt::Horizontal)
			r.adjust(-half, 0, half, 0);
		else
			r.adjust(0, -half / 2, 0, half / 2);
		bounds.append(r);
	}

	const int midLabelPos = allTicks.size() / 2;

	// Decimate by growing a stride until the kept labels no longer overlap.
	QList<double> ticks;
	for(int stride = 1;; ++stride) {
		QList<int> keep;
		if(m_delta) {
			keep.append(midLabelPos);
			for(int i = midLabelPos - stride; i >= 0; i -= stride)
				keep.prepend(i);
			for(int i = midLabelPos + stride; i < allTicks.size(); i += stride)
				keep.append(i);
		} else {
			for(int i = 0; i < allTicks.size(); i += stride)
				keep.append(i);
			if(keep.last() != allTicks.size() - 1)
				keep.append(allTicks.size() - 1);
		}

		bool overlap = false;
		for(int k = 1; k < keep.size(); ++k) {
			if(bounds.at(keep.at(k)).intersects(bounds.at(keep.at(k - 1)))) {
				overlap = true;
				break;
			}
		}

		if(!overlap || keep.size() <= 2) {
			for(int idx : std::as_const(keep))
				ticks.append(allTicks.at(idx));
			break;
		}
	}

	double delta = -INFINITY;

	if(m_delta && m_nrTicks > midLabelPos) {
		delta = allTicks[midLabelPos];
		drawLabel(painter, delta);
	}

	for(const auto &tick : std::as_const(ticks)) {
		if(tick != delta) {
			drawLabel(painter, tick);
		}
	}
}

QwtText BasicScaleDraw::label(double value) const
{
	QString prefix;
	double scale = 1.0;
	QString sign = "";
	int bonusPrecision = 0;
	bool center = false;

	double lower = scaleDiv().interval().minValue();
	double upper = scaleDiv().interval().maxValue();
	double diff = upper - lower;
	double step = diff / (m_nrTicks ? (m_nrTicks - 1) : 1);

	int mid = (m_nrTicks / 2 + 1);

	if(m_delta) {
		int current = 0;
		while(value > (lower + current * step))
			current++;
		int position = current + 1;

		if(position == mid) {
			// center label with extra precision
			center = true;
			bonusPrecision = 1;
		} else if(position < mid) {
			sign = "-";
			// negative delta label
			value = step * (mid - position);
		} else if(position > mid) {
			sign = "+";
			// positive delta label
			value = step * (position - mid);
		}
	}

	value *= m_displayScale;

	QwtText text;
	QString unit = "";
	if(m_unitsEn) {
		if(m_formatter) {
			m_formatter->getFormatAttributes(value, prefix, scale);
		}

		if(orientation() == Qt::Vertical) {
			double absVal = value > 0 ? value : -value;
			if(absVal > 1e-2 && prefix == "m") {
				scale = 1.0;
				prefix = "";
			} else if(absVal > 1e-5 && prefix == "μ") {
				scale = 1e-3;
				prefix = "m";
			} else if(absVal > 1e-8 && prefix == "n") {
				scale = 1e-6;
				prefix = "μ";
			} else if(absVal > 1e-11 && prefix == "p") {
				scale = 1e-9;
				prefix = "n";
			}
		}

		/*text = QwtText(sign + QLocale().toString(value / scale, 'f', m_floatPrecision + bonusPrecision) + ' '
		   + prefix + m_unit);*/
		unit = m_unit;
	}
	text = QwtText(m_formatter->format(value, unit, m_floatPrecision + bonusPrecision));

	if(m_color != Qt::gray)
		text.setColor(m_color);
	if(center) {
		text.setColor(QColor(255, 255, 255));
	}

	return text;
}
