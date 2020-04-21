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

#include "osc_adjuster.hpp"

#include <qdebug.h>
#include <qevent.h>

OscAdjuster::OscAdjuster(QWidget *parent, QwtAxisId axisId)
	: QObject(parent)
	, d_isEnabled(false)
	, d_mousePressed(false)
	, d_mouseFactor(0.95)
	, d_mouseButton(Qt::LeftButton)
	, d_axisId(axisId) {
	setEnabled(true);
}

OscAdjuster::~OscAdjuster() {}

QWidget *OscAdjuster::parentWidget() { return qobject_cast<QWidget *>(parent()); }

const QWidget *OscAdjuster::parentWidget() const { return qobject_cast<const QWidget *>(parent()); }

QwtAxisId OscAdjuster::axisId() { return d_axisId; }

void OscAdjuster::setMouseFactor(double factor) { d_mouseFactor = factor; }

double OscAdjuster::mouseFactor() const { return d_mouseFactor; }

void OscAdjuster::setEnabled(bool on) {
	if (d_isEnabled != on) {
		d_isEnabled = on;

		QObject *o = parent();
		if (o) {
			if (d_isEnabled)
				o->installEventFilter(this);
			else
				o->removeEventFilter(this);
		}
	}
}

bool OscAdjuster::isEnabled() const { return d_isEnabled; }

void OscAdjuster::setMouseButton(Qt::MouseButton button) { d_mouseButton = button; }

void OscAdjuster::getMouseButton(Qt::MouseButton &button) const { button = d_mouseButton; }

bool OscAdjuster::eventFilter(QObject *object, QEvent *event) {
	if (object && object == parent()) {
		switch (event->type()) {
		case QEvent::MouseButtonPress: {
			widgetMousePressEvent(static_cast<QMouseEvent *>(event));
			break;
		}
		case QEvent::MouseMove: {
			widgetMouseMoveEvent(static_cast<QMouseEvent *>(event));
			break;
		}
		case QEvent::MouseButtonRelease: {
			widgetMouseReleaseEvent(static_cast<QMouseEvent *>(event));
			break;
		}
		case QEvent::Wheel: {
			widgetMouseWheelEvent(static_cast<QWheelEvent *>(event));
			break;
		}
		default:;
		}
	}
	return QObject::eventFilter(object, event);
}

void OscAdjuster::widgetMousePressEvent(QMouseEvent *mouseEvent) {
	if (parentWidget() == NULL)
		return;

	if (mouseEvent->button() != d_mouseButton) {
		return;
	}

	d_hasMouseTracking = parentWidget()->hasMouseTracking();

	parentWidget()->setMouseTracking(true);
	d_mousePos = mouseEvent->pos();
	d_mousePressed = true;
}

void OscAdjuster::widgetMouseReleaseEvent(QMouseEvent *mouseEvent) {
	Q_UNUSED(mouseEvent);

	if (d_mousePressed && parentWidget()) {
		d_mousePressed = false;
		parentWidget()->setMouseTracking(d_hasMouseTracking);
	}
}

void OscAdjuster::widgetMouseWheelEvent(QWheelEvent *wheelEvent) {
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
