/*
 * Copyright 2016 Analog Devices, Inc.
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

#include "completion_circle.h"

#include <QMouseEvent>
#include <QPainter>
#include <qmath.h>

#include <QDebug>

using namespace adiscope;

CompletionCircle::CompletionCircle(QWidget *parent, bool invert_circle) :
	QDial(parent), m_xc(0), m_yc(0), m_radius(23),
	m_pressed(false), m_log_scale(false), invert_circle(invert_circle), m_origin(90)
{
	setWrapping(true);
	setMinimumSize(50, 50);
	setMaximumSize(50, 50);
}

void CompletionCircle::paintEvent(QPaintEvent *)
{
	QPainter p(this);

	double min;
	double max;
	double val;
	double completeRatio;
	double angle;
	int r = m_radius;
	int xc = width() / 2;
	int yc = height() / 2;

	m_xc = xc;
	m_yc = yc;

	if (!m_log_scale) {
		min = minimumDouble();
		max = maximumDouble();
		val = valueDouble();
	} else {
		min = log(minimumDouble());
		max = log(maximumDouble());
		val = log(valueDouble());
	}

	completeRatio = qAbs((val - min) / (max - min));

	if (invert_circle) {
		completeRatio = 1.0 - completeRatio;
	}

	angle = completeRatio * 360;

	QColor qss_bgcolor(p.background().color());
	QColor qss_color(p.pen().color());

	p.setRenderHint(QPainter::Antialiasing);

	// The color filled rail
	QPen rail_pen(qss_color);
	rail_pen.setWidth(3);
	p.setPen(rail_pen);
	QRectF rect(xc - r, yc - r, 2* r, 2 * r);
	p.drawArc(rect, m_origin * 16, angle * 16);

	// Background circle
	QPen bg_pen(qss_bgcolor);
	bg_pen.setWidth(3);
	p.setPen(bg_pen);
	p.drawArc(rect, (m_origin + angle) * 16, (360 - angle) * 16);

	// The center dot
	QColor centerDotColor(Qt::black);

	if (m_pressed) {
		centerDotColor = QColor(255, 114, 0);
	}

	p.setPen(centerDotColor);
	p.setBrush(centerDotColor);
	p.drawEllipse(QPoint(xc, yc), 3, 3);

	// The dash
	angle = angle + m_origin;

	double rad_angle = qDegreesToRadians(angle);

	int x1 = (r - 11) * qCos(rad_angle);
	int y1 = (r - 11) * qSin(rad_angle);

	int x2 = (r - 6) * qCos(rad_angle);
	int y2 = (r - 6) * qSin(rad_angle);

	QPen pen(Qt::white);
	pen.setStyle(Qt::SolidLine);
	pen.setWidthF(1.5);
	p.setPen(pen);
	p.drawLine(xc + x1, yc - y1, xc + x2, yc - y2);
}

void CompletionCircle::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton) {
		if (pointInsideCircle(e->pos(), m_xc, m_yc, m_radius - 2)) {
			m_pressed = !m_pressed;
			repaint();
			Q_EMIT toggled(m_pressed);
		}
	}
}

void CompletionCircle::mouseReleaseEvent(QMouseEvent *)
{
}

void CompletionCircle::mouseMoveEvent(QMouseEvent *)
{
}

void CompletionCircle::keyPressEvent(QKeyEvent *)
{
}

void CompletionCircle::wheelEvent(QWheelEvent *)
{
}

bool CompletionCircle::pointInsideCircle(const QPoint& p, int xc, int yc, int r)
{
	int x = p.x();
	int y = p.y();

	return ((x - xc) * (x - xc) + (y - yc) * (y - yc) < r * r);
}

double CompletionCircle::valueDouble()
{
	return m_double_value;
}

void CompletionCircle::setValueDouble(double value)
{
	if (value < m_double_minimum) {
		value = m_double_minimum;
	} else if (value > m_double_maximum) {
		value = m_double_maximum;
	}

	if (m_double_value != value) {
		m_double_value = value;
		sliderChange(SliderValueChange);
	}
}

bool CompletionCircle::toggledState()
{
	return m_pressed;
}

void CompletionCircle::setToggled(bool on)
{
	if (m_pressed != on) {
		m_pressed = on;
		Q_EMIT toggled(on);
	}
}

double CompletionCircle::minimumDouble()
{
	return m_double_minimum;
}

void CompletionCircle::setOrigin(double value)
{
	m_origin = value;
}

void CompletionCircle::setMinimumDouble(double value)
{
	m_double_minimum = value;

	if (m_double_value < m_double_minimum) {
		setValueDouble(value);
	}

	repaint();
}

double CompletionCircle::maximumDouble()
{
	return m_double_maximum;
}

void CompletionCircle::setMaximumDouble(double value)
{
	m_double_maximum = value;

	if (m_double_value > m_double_maximum) {
		setValueDouble(value);
	}

	repaint();
}

bool CompletionCircle::isLogScale()
{
	return m_log_scale;
}

void CompletionCircle::setIsLogScale(bool state)
{
	m_log_scale = state;
}
