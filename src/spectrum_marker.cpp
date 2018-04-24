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

#include "spectrum_marker.hpp"

#include <qwt_plot.h>
#include <qwt_symbol.h>

using namespace adiscope;

SpectrumMarker::SpectrumMarker(const QString &title, bool movable) :
	QwtPlotMarker(title), m_xAxis(QwtPlot::xBottom),
	m_yAxis(QwtPlot::yLeft), m_movable(movable), m_selected(false)
{
}

SpectrumMarker::SpectrumMarker(const QString &title, QwtAxisId xAxis,
	QwtAxisId yAxis, bool movable) :
	QwtPlotMarker(title), m_xAxis(xAxis), m_yAxis(yAxis), m_movable(movable), m_selected(false)
{
}

SpectrumMarker::~SpectrumMarker()
{
}

QColor SpectrumMarker::defaultColor() const
{
	return m_default_color;
}

void SpectrumMarker::setDefaultColor(const QColor& color)
{
	m_default_color = color;
}

bool SpectrumMarker::selected() const
{
	return m_selected;
}

void SpectrumMarker::setSelected(bool on)
{
	if (m_selected != on) {
		m_selected = on;

		QBrush brush = symbol()->brush();
		if (on) {
			brush.setColor(defaultColor());
		} else {
			brush.setColor(symbol()->pen().color());
		}

		QwtSymbol *new_sym = new QwtSymbol(symbol()->style(),
			brush, symbol()->pen(), symbol()->size());
		setSymbol(new_sym);
	}
}

bool SpectrumMarker::movable() const
{
	return m_movable;
}

void SpectrumMarker::setMovable(bool on)
{
	m_movable = on;
}

QPoint SpectrumMarker::plotValueToPixels(const QPointF& point) const
{
	QPoint pixelPoint(0, 0);

	if (plot()) {
		const QwtScaleMap xMap = plot()->canvasMap(m_xAxis);
		const QwtScaleMap yMap = plot()->canvasMap(m_yAxis);
		QPointF p = QwtScaleMap::transform(xMap, yMap, point);
		pixelPoint = QPoint(p.x(),p.y());
	}

	return pixelPoint;
}

QPointF SpectrumMarker::plotPixelsToValue(const QPoint& point) const
{
	QPointF valuePoint(0.0, 0.0);

	if (plot()) {
		const QwtScaleMap xMap = plot()->canvasMap(m_xAxis);
		const QwtScaleMap yMap = plot()->canvasMap(m_yAxis);

		valuePoint = QwtScaleMap::invTransform(xMap, yMap, point);
	}

	return valuePoint;
}

QRect SpectrumMarker::boundingRectPixels() const
{
	QPoint markerPixPos = plotValueToPixels(value());
	QRect symbolRect = symbol()->boundingRect();


	return QRect(symbolRect.topLeft() + markerPixPos, symbolRect.size());
}
