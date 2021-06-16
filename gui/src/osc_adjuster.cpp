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

#include <qdebug.h>
#include <qevent.h>

#include <scopy/gui/osc_adjuster.hpp>

OscAdjuster::OscAdjuster(QWidget* parent, QwtAxisId axisId)
	: QObject(parent)
	, m_isEnabled(false)
	, m_mousePressed(false)
	, m_mouseFactor(0.95)
	, m_mouseButton(Qt::LeftButton)
	, m_axisId(axisId)
{
	setEnabled(true);
}

OscAdjuster::~OscAdjuster() {}

QWidget* OscAdjuster::parentWidget() { return qobject_cast<QWidget*>(parent()); }

const QWidget* OscAdjuster::parentWidget() const { return qobject_cast<const QWidget*>(parent()); }

QwtAxisId OscAdjuster::axisId() { return m_axisId; }

void OscAdjuster::setMouseFactor(double factor) { m_mouseFactor = factor; }

double OscAdjuster::mouseFactor() const { return m_mouseFactor; }

void OscAdjuster::setEnabled(bool on)
{
	if (m_isEnabled != on) {
		m_isEnabled = on;

		QObject* o = parent();
		if (o) {
			if (m_isEnabled)
				o->installEventFilter(this);
			else
				o->removeEventFilter(this);
		}
	}
}

bool OscAdjuster::isEnabled() const { return m_isEnabled; }

void OscAdjuster::setMouseButton(Qt::MouseButton button) { m_mouseButton = button; }

void OscAdjuster::getMouseButton(Qt::MouseButton& button) const { button = m_mouseButton; }

bool OscAdjuster::eventFilter(QObject* object, QEvent* event)
{
	if (object && object == parent()) {
		switch (event->type()) {
		case QEvent::MouseButtonPress: {
			widgetMousePressEvent(static_cast<QMouseEvent*>(event));
			break;
		}
		case QEvent::MouseMove: {
			widgetMouseMoveEvent(static_cast<QMouseEvent*>(event));
			break;
		}
		case QEvent::MouseButtonRelease: {
			widgetMouseReleaseEvent(static_cast<QMouseEvent*>(event));
			break;
		}
		case QEvent::Wheel: {
			widgetMouseWheelEvent(static_cast<QWheelEvent*>(event));
			break;
		}
		default:;
		}
	}
	return QObject::eventFilter(object, event);
}

void OscAdjuster::widgetMousePressEvent(QMouseEvent* mouseEvent)
{
	if (parentWidget() == NULL)
		return;

	if (mouseEvent->button() != m_mouseButton) {
		return;
	}

	m_hasMouseTracking = parentWidget()->hasMouseTracking();

	parentWidget()->setMouseTracking(true);
	m_mousePos = mouseEvent->pos();
	m_mousePressed = true;
}

void OscAdjuster::widgetMouseReleaseEvent(QMouseEvent* mouseEvent)
{
	Q_UNUSED(mouseEvent);

	if (m_mousePressed && parentWidget()) {
		m_mousePressed = false;
		parentWidget()->setMouseTracking(m_hasMouseTracking);
	}
}

void OscAdjuster::widgetMouseWheelEvent(QWheelEvent* wheelEvent)
{
	const int y = wheelEvent->angleDelta().y();

	// Only y value changes if a mouse with wheel is used.
	// TO DO: Check if this works for touch screens and fix if necessary
	if (y != 0) {
		if (y > 0)
			Q_EMIT wheelUp(y);
		else
			Q_EMIT wheelDown(y);
	}
}
