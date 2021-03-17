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
#include "osc_custom_scroll.hpp"

#include <QApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QPointF>
#include <QScrollBar>

using namespace scopy::gui;

OscCustomScrollArea::OscCustomScrollArea(QWidget* parent)
	: QScrollArea(parent)
	, m_inside(false)
	, m_disableCursor(true)
{
	QScroller::grabGesture(this->viewport(), QScroller::LeftMouseButtonGesture);

	m_scroll = QScroller::scroller(this->viewport());

	QScrollerProperties properties = QScroller::scroller(this->viewport())->scrollerProperties();

	QVariant overshootPolicy =
		QVariant::fromValue<QScrollerProperties::OvershootPolicy>(QScrollerProperties::OvershootWhenScrollable);
	properties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, overshootPolicy);

	QScroller::scroller(this->viewport())->setScrollerProperties(properties);

	connect(m_scroll, &QScroller::stateChanged, [=](QScroller::State newstate) {
		if (m_disableCursor)
			return;
		switch (newstate) {
		case QScroller::Inactive:
			if (m_inside)
				setCursor(Qt::OpenHandCursor);
			break;
		case QScroller::Pressed:
			setCursor(Qt::ClosedHandCursor);
			break;
		case QScroller::Dragging:
			setCursor(Qt::ClosedHandCursor);
			break;
		case QScroller::Scrolling:
			if (m_inside)
				setCursor(Qt::OpenHandCursor);
			break;
		default:
			setCursor(Qt::ArrowCursor);
			break;
		}
	});

	connect(horizontalScrollBar(), &QScrollBar::rangeChanged, [=](int v1, int v2) {
		if (v2 - v1 == 0)
			m_disableCursor = true;
		else
			m_disableCursor = false;
	});
}

OscCustomScrollArea::~OscCustomScrollArea() { QScroller::ungrabGesture(this->viewport()); }

void OscCustomScrollArea::enterEvent(QEvent* event)
{
	if (!m_disableCursor)
		setCursor(Qt::OpenHandCursor);
	m_inside = true;
}

void OscCustomScrollArea::leaveEvent(QEvent* event)
{
	if (!m_disableCursor)
		setCursor(Qt::ArrowCursor);
	m_inside = false;
}
