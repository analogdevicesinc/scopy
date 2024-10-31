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

#include "axishandle.h"
#include "qcoreevent.h"
#include "qpainter.h"
#include <qwt_scale_div.h>
#include <QMouseEvent>

using namespace scopy;

AxisHandle::AxisHandle(QwtAxisId axisId, HandlePos handlePos, QwtPlot *plot)
	: QWidget(plot->canvas())
	, m_axisId(axisId)
	, m_plot(plot)
	, m_color(Qt::gray)
	, m_handlePos(handlePos)
	, m_pos(QPoint(0, 0))
	, m_handleMargins(6)
	, m_handlePadding(3)
	, m_handleSize(17)
	, m_isHovering(false)
	, m_pressed(false)
	, m_bounded(true)
	, m_barVisibility(BarVisibility::ALWAYS)
{
	init();
}

AxisHandle::~AxisHandle() {}

uint AxisHandle::getLineWidth() { return m_pen.width(); }

void AxisHandle::setLineWidth(int width) { m_pen.setWidth(width); }

QColor AxisHandle::getColor() { return m_color; }

QwtAxisId AxisHandle::getAxisId() { return m_axisId; }

int AxisHandle::getPos() { return m_axisId.isXAxis() ? m_pos.x() : m_pos.y(); }

void AxisHandle::setPosSilent(int pos) { m_axisId.isXAxis() ? m_pos.setX(pos) : m_pos.setY(pos); }

void AxisHandle::setBarVisibility(BarVisibility bar) { m_barVisibility = bar; }

BarVisibility AxisHandle::getBarVisibility() { return m_barVisibility; }

HandlePos AxisHandle::getHandlePos() { return m_handlePos; }

void AxisHandle::setBounded(bool bounded) { m_bounded = bounded; }

bool AxisHandle::isBounded() const { return m_bounded; }

void AxisHandle::setAxis(QwtAxisId axis)
{
	m_axisId = axis;
	repaint();
}

void AxisHandle::setHandlePos(HandlePos pos)
{
	m_handlePos = pos;
	updateHandleOrientation();
}

void AxisHandle::setHandle(HandleOrientation orientation)
{
	m_handle = QPixmap(":/gui/icons/handle_arrow.svg")
			   .scaled(m_handleSize, m_handleSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	if(m_handlePos == HandlePos::NORTH_OR_WEST)
		m_handle = m_handle.transformed(QTransform().scale(-1, -1));

	if(m_axisId.isYAxis())
		m_handle = m_handle.transformed(QTransform().rotate(-90));

	if(orientation != HandleOrientation::TO_CENTER) {
		if(orientation == HandleOrientation::TO_MIN) {
			m_handle = m_handle.transformed(QTransform().rotate(-90));
		}
		if(orientation == HandleOrientation::TO_MAX) {
			m_handle = m_handle.transformed(QTransform().rotate(90));
		}

		if(m_axisId.isYAxis()) {
			m_handle = m_handle.transformed(QTransform().rotate(180));
		}

		if(m_handlePos == HandlePos::NORTH_OR_WEST) {
			m_handle = m_handle.transformed(QTransform().rotate(180));
		}
	}
}

void AxisHandle::init()
{
	installEventFilter(this);
	m_plot->canvas()->installEventFilter(this);
	setAttribute(Qt::WA_TransparentForMouseEvents, false);

	m_pen = QPen(m_color, 1, Qt::DashLine);
	setHandle();
}

void AxisHandle::setColor(const QColor &color)
{
	m_color = color;
	m_pen.setColor(color);
	repaint();
}

void AxisHandle::setPos(int pos)
{
	setPosSilent(pos);
	Q_EMIT pixelPosChanged(pos);
	repaint();
}

QRect AxisHandle::getBigRect()
{
	QRect rect;
	int rectWidth = m_handle.width() + m_handleMargins * 2;
	int rectHeight = m_handle.height() + m_handleMargins * 2;

	if(m_axisId.isXAxis()) {
		double x =
			std::min(std::max(m_pos.x() - m_handleMargins - m_handle.width() / 2, 0), width() - rectWidth);
		rect = QRect(x, m_pos.y() - m_handleMargins * 2, rectWidth, rectHeight);
	} else {
		double y = std::min(std::max(m_pos.y() - m_handleMargins - m_handle.height() / 2, 0),
				    height() - rectHeight);
		rect = QRect(m_pos.x() - m_handleMargins * 2, y, rectWidth, rectHeight);
	}

	if(m_handlePos == HandlePos::NORTH_OR_WEST) {
		if(m_axisId.isXAxis()) {
			int height = rect.height();
			rect.setTop(0);
			rect.setBottom(height);
		} else {
			int width = rect.width();
			rect.setLeft(0);
			rect.setWidth(width);
		}
	}
	return rect;
}

QRect AxisHandle::getSmallRect()
{
	QRect rect;
	if(m_axisId.isXAxis()) {
		double x = std::min(std::max(m_pos.x() - m_handle.width() / 2, 0), width() - m_handle.width());
		rect = QRect(x, m_pos.y() - m_handlePadding, m_handle.width(), m_handle.height());
	} else {
		double y = std::min(std::max(m_pos.y() - m_handle.height() / 2, 0), height() - m_handle.height());
		rect = QRect(m_pos.x() - m_handlePadding, y, m_handle.width(), m_handle.height());
	}

	if(m_handlePos == HandlePos::NORTH_OR_WEST) {
		if(m_axisId.isXAxis()) {
			int height = rect.height();
			rect.setTop(m_handlePadding);
			rect.setBottom(height);
		} else {
			int width = rect.width();
			rect.setLeft(m_handlePadding);
			rect.setWidth(width);
		}
	}
	return rect;
}

QRect AxisHandle::getRect() { return m_isHovering ? getBigRect() : getSmallRect(); }

QLine AxisHandle::getLine()
{
	QLine line;
	if(m_axisId.isXAxis()) {
		line = QLine(m_pos.x(), height(), m_pos.x(), 0);
	} else {
		line = QLine(width(), m_pos.y(), 0, m_pos.y());
	}
	return line;
}

QRect AxisHandle::getRectFromLine(QLine line)
{
	QRect rect;
	if(m_axisId.isXAxis()) {
		rect = QRect(QPoint(line.x1() - m_pen.width(), line.y2()),
			     QPoint(line.x2() + m_pen.width(), line.y1()));
	} else {
		rect = QRect(QPoint(line.x2(), line.y1() - m_pen.width()),
			     QPoint(line.x1(), line.y2() + m_pen.width()));
	}
	return rect;
}

void AxisHandle::updateHandleOrientation()
{
	double x = getBigRect().x();
	double y = getBigRect().y();
	int rectWidth = getBigRect().width();
	int rectHeight = getBigRect().height();

	if(m_axisId.isXAxis()) {
		if(x == 0) {
			setHandle(HandleOrientation::TO_MIN);
		} else if(x == width() - rectWidth) {
			setHandle(HandleOrientation::TO_MAX);
		} else {
			setHandle(HandleOrientation::TO_CENTER);
		}
	} else {
		if(y == 0) {
			setHandle(HandleOrientation::TO_MIN);
		} else if(y == height() - rectHeight) {
			setHandle(HandleOrientation::TO_MAX);
		} else {
			setHandle(HandleOrientation::TO_CENTER);
		}
	}
}

void AxisHandle::paintEvent(QPaintEvent *event)
{
	QPainter p(this);
	p.setBrush(m_color);
	p.setPen(m_pen);
	p.setRenderHint(QPainter::Antialiasing);

	QLine line = getLine();
	if(m_barVisibility == BarVisibility::ALWAYS || (m_barVisibility == BarVisibility::ON_HOVER && m_isHovering)) {
		p.drawLine(line);
		setMask(QRegion(getRectFromLine(line)) + QRegion(getBigRect()));
	} else {
		setMask(QRegion(getBigRect()));
	}

	QRect rect = getRect();
	double rad = 50.;
	p.setPen(QPen());
	p.drawRoundedRect(rect, rad, rad, Qt::RelativeSize);
	updateHandleOrientation();
	p.drawPixmap(QPoint(rect.center().x() - m_handle.width() / 2, rect.center().y() - m_handle.height() / 2),
		     m_handle);
}

void AxisHandle::onMouseMove(QPointF pos)
{
	if(m_axisId.isXAxis()) {
		if(m_bounded) {
			setPos(std::min(m_plot->canvas()->width() - 1., std::max(pos.x(), 0.)));
		} else {
			setPos(pos.x());
		}
	} else {
		if(m_bounded) {
			setPos(std::min(m_plot->canvas()->height() - 1., std::max(pos.y(), 0.)));
		} else {
			setPos(pos.y());
		}
	}
}

bool AxisHandle::onMouseButtonPress(QPointF pos)
{
	if(getBigRect().contains(pos.toPoint())) {
		m_isHovering = true;
		m_pressed = true;
		setCursor(Qt::ClosedHandCursor);
		return true;
	}
	return false;
}

void AxisHandle::onMouseButtonRelease()
{
	if(m_isHovering) {
		m_pressed = false;
	}
	setCursor(Qt::OpenHandCursor);
}

bool AxisHandle::onEnter(QPointF pos)
{
	if(getBigRect().contains(pos.toPoint())) {
		m_isHovering = true;
		repaint();
		setCursor(Qt::OpenHandCursor);
		return true;
	}
	return false;
}

void AxisHandle::onLeave()
{
	m_isHovering = false;
	repaint();
	setCursor(Qt::ArrowCursor);
}

void AxisHandle::onResize()
{
	setFixedSize(QSize(m_plot->canvas()->size()));
	if(m_axisId.isXAxis()) {
		m_pos.setY(m_plot->canvas()->height() - m_handle.height());
	} else {
		m_pos.setX(m_plot->canvas()->width() - m_handle.width());
	}
}

void AxisHandle::onDoubleClick(QPointF pos)
{
	if(getBigRect().contains(pos.toPoint())) {
		if(m_axisId.isXAxis()) {
			setPos(m_plot->canvas()->width() / 2);
		} else {
			setPos(m_plot->canvas()->height() / 2);
		}
	}
}

bool AxisHandle::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonRelease) {
		onMouseButtonRelease();

	} else if(event->type() == QEvent::Leave) {
		onLeave();

	} else if(event->type() == QEvent::MouseButtonDblClick) {
		onDoubleClick(dynamic_cast<QMouseEvent *>(event)->pos());

	} else if(event->type() == QEvent::Resize) {
		onResize();

	} else if(event->type() == QEvent::Enter) {
		if(onEnter(dynamic_cast<QEnterEvent *>(event)->pos())) {
			return true;
		}
	} else if(event->type() == QEvent::MouseButtonPress) {
		if(onMouseButtonPress(dynamic_cast<QMouseEvent *>(event)->pos())) {
			return true;
		}
	}
	if(event->type() == QEvent::MouseMove && m_pressed) {
		onMouseMove(dynamic_cast<QMouseEvent *>(event)->pos());
		return true;
	}

	return QObject::eventFilter(object, event);
}

#include "moc_axishandle.cpp"
