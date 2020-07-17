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

#include "plot_line_handle.h"
#include "handles_area.hpp"

#include <QPainter>
#include <QMoveEvent>

#include <QApplication>

#include <QDebug>

PlotLineHandle::PlotLineHandle(const QPixmap &handleIcon, QWidget *parent):
	QWidget(parent),
	m_enable_silent_move(false),
	m_innerSpacing(0),
	m_outerSpacing(0),
	m_image(handleIcon),
	m_grabbed(false),
	m_current_pos(0),
	m_width(20),
	m_height(20)
{
}

void PlotLineHandle::moveSilently(QPoint pos)
{
	m_enable_silent_move = true;
	moveWithinParent(pos.x(), pos.y());
}

int PlotLineHandle::position()
{
	return m_current_pos;
}

void PlotLineHandle::setPen(const QPen& pen)
{
	m_pen = pen;
}

const QPen& PlotLineHandle::pen()
{
	return m_pen;
}

void PlotLineHandle::enterEvent(QEvent *event)
{
	setCursor(Qt::OpenHandCursor);
	QWidget::enterEvent(event);
}

void PlotLineHandle::mousePressEvent(QMouseEvent *event)
{
	QWidget *parent = static_cast<QWidget *>(this->parent());

	setCursor(Qt::ClosedHandCursor);
	parent->raise();

	QWidget::mousePressEvent(event);
}

void PlotLineHandle::mouseReleaseEvent(QMouseEvent *event)
{
	Q_EMIT mouseReleased();
	setCursor(Qt::OpenHandCursor);
	QWidget::mouseReleaseEvent(event);
}

void PlotLineHandle::setGrabbed(bool grabbed)
{
	if (m_grabbed != grabbed) {
		m_grabbed = grabbed;
		Q_EMIT grabbedChanged(grabbed);
	}
}

PlotGateHandle::PlotGateHandle(const QPixmap &handleIcon, QWidget *parent):
	PlotLineHandle(handleIcon, parent),
	m_alignLeft(true),
	m_reachLimit(false),
	m_timeValue(0)
{
	m_width = m_image.width();
	m_height = m_image.height();
	setMinimumSize(m_width, m_height);
	setMaximumSize(m_width, m_height);
}

void PlotGateHandle::triggerMove()
{
	Q_EMIT positionChanged(m_current_pos);
}

void PlotGateHandle::setPosition(int pos)
{
	if (m_current_pos != pos)
		moveWithinParent(centerPosToOrigin(pos), 0);
}

void PlotGateHandle::setPositionSilenty(int pos)
{
	if (m_current_pos != pos)
		moveSilently(QPoint(centerPosToOrigin(pos), 0));
}

void PlotGateHandle::updatePosition()
{
	moveSilently(QPoint(0, centerPosToOrigin(m_current_pos)));
}

void PlotGateHandle::moveWithinParent(int x, int y)
{
	Q_UNUSED(y);

	HorizHandlesArea *area = static_cast<HorizHandlesArea *>(parent());

	int lower_limit = 0 + area->leftPadding() - width() / 2;
	int upper_limit = area->width() - area->rightPadding() - width() / 2 - 1;

	int initialX = x;
	if (x < lower_limit && !m_alignLeft){
		x = lower_limit;
		m_reachLimit = true;//handle hit the margin
	}
	else if (x > upper_limit && m_alignLeft){
		x = upper_limit;
		m_reachLimit = true;//handle hit the margin
	}
	else{
		m_reachLimit = false;
	}

	int centerPos = originPosToCenter(x);
	int oldCenterPos = originPosToCenter(this->x());

	int oldCurrentPos = m_current_pos;

	if(m_reachLimit){
		m_position = initialX;
	}
	else{
		m_current_pos = centerPos;
	}

	if (centerPos != oldCenterPos) {
		if(!m_reachLimit){
			if(x < m_otherCursorPos + width() && !m_alignLeft){
				/*move the handle only if it doens't hit the other handle */
				move(x, this->y());
			}
			else if(x > m_otherCursorPos && m_alignLeft){
				/*move the handle only if it doens't hit the other handle */
				move(x,this->y());
			}
			else{/* remember bar position */
				centerPos = oldCurrentPos;
				m_current_pos = centerPos;
			}
		}
		else{
			if(m_alignLeft){//make sure that the handle doesn't go outside of the plot margins
				move(upper_limit,this->y());
			}
			else{
				move(lower_limit,this->y());
			}
		}
		if (!m_enable_silent_move)
			Q_EMIT positionChanged(centerPos);

	}
	else{
		if(m_reachLimit){//update only the bar position if the handle reached a margin
			Q_EMIT positionChanged(originPosToCenter(m_position));
		}
	}
	m_enable_silent_move = false;
}

void PlotGateHandle::setInnerSpacing(int value)
{
	m_innerSpacing = value;
}

int PlotGateHandle::position()
{
	if(m_reachLimit)//if the handle reached the limit return the position of the bar
		return originPosToCenter(m_position);
	return m_current_pos;
}

void PlotGateHandle::setTimeValue(double val)
{
	m_timeValue = val;
	repaint();
}

void PlotGateHandle::setCenterLeft(bool val)
{/* align the bar to the left or the right part of the handle*/
	m_alignLeft = val;
}

int PlotGateHandle::getCurrentPos()
{
	return m_position;
}

bool PlotGateHandle::reachedLimit()
{
	return m_reachLimit;//cursor handle reached the plot margin
}

void PlotGateHandle::setOtherCursorPosition(int position)
{
	m_otherCursorPos = position;//position of the other handle cursor
}

void PlotGateHandle::paintEvent(QPaintEvent *event)
{
	QPainter p(this);
	QPoint imageTopLeft;

	imageTopLeft = QPoint(0, 0);
	p.drawPixmap(imageTopLeft, m_image);

	p.setPen(QPen(QColor(Qt::black)));
	QString handleText = d_timeFormatter.format(m_timeValue,"",2);

	QFontMetrics fm = p.fontMetrics();
	int textWidth = fm.width(handleText);
	int textHeight = fm.height();

	p.drawText(QPoint((width()-textWidth)/2,height()-textHeight/2),handleText);
}

int PlotGateHandle::originPosToCenter(int origin)
{
	HorizHandlesArea *area = static_cast<HorizHandlesArea *>(parent());
	int offset = -area->leftPadding() + width() / 2 + (m_alignLeft ? 0 : width());

	return (origin + offset);
}

int PlotGateHandle::centerPosToOrigin(int center)
{
	HorizHandlesArea *area = static_cast<HorizHandlesArea *>(parent());
	int offset = -area->leftPadding() + width() / 2 + (m_alignLeft ? 0 : width());

	return (center - offset);
}


PlotLineHandleH::PlotLineHandleH(const QPixmap &handleIcon, QWidget *parent,
			bool facingBottom):
	PlotLineHandle(handleIcon, parent),
	m_facingBottom(facingBottom)
{
	m_innerSpacing = m_image.width() / 2;
	m_width = m_image.width();
	m_height = m_innerSpacing + m_image.height() + m_outerSpacing;
	setMinimumSize(m_width, m_height);
	setMaximumSize(m_width, m_height);
}

void PlotLineHandleH::triggerMove()
{
	Q_EMIT positionChanged(m_current_pos);
}

void PlotLineHandleH::setPosition(int pos)
{
	if (m_current_pos != pos)
		moveWithinParent(centerPosToOrigin(pos), 0);
}

void PlotLineHandleH::setPositionSilenty(int pos)
{
	if (m_current_pos != pos)
		moveSilently(QPoint(centerPosToOrigin(pos), 0));
}

void PlotLineHandleH::updatePosition()
{
	moveSilently(QPoint(centerPosToOrigin(m_current_pos), 0));
}

void PlotLineHandleH::setInnerSpacing(int value)
{
	m_innerSpacing = value;
}

void PlotLineHandleH::moveWithinParent(int x, int y)
{
	Q_UNUSED(y);

	HorizHandlesArea *area = static_cast<HorizHandlesArea *>(parent());

	int lower_limit = 0 + area->leftPadding() - width() / 2;
	int upper_limit = area->width() - area->rightPadding() - width() / 2 - 1;

	if (x < lower_limit)
		x = lower_limit;
	else if (x > upper_limit)
		x = upper_limit;

	int centerPos = originPosToCenter(x);
	int oldCenterPos = originPosToCenter(this->x());

	if (centerPos != oldCenterPos) {
		move(x, this->y());

		if (!m_enable_silent_move)
			Q_EMIT positionChanged(centerPos);
	}
	m_enable_silent_move = false;
	m_current_pos = centerPos;
}

void PlotLineHandleH::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QPoint lineStart;
	QPoint imageTopLeft;

	p.setPen(m_pen);

	if (m_facingBottom) {
		lineStart = QPoint(m_image.width() / 2,
				m_image.height());
		imageTopLeft = QPoint(0, 0);
	} else {
		lineStart = QPoint(m_image.width() / 2, 0);
		imageTopLeft = QPoint(0, m_innerSpacing);
	}

	p.drawLine(lineStart, lineStart + QPoint(0, m_innerSpacing));
	p.drawPixmap(imageTopLeft, m_image);
}

int PlotLineHandleH::originPosToCenter(int origin)
{
	HorizHandlesArea *area = static_cast<HorizHandlesArea *>(parent());
	int offset = -area->leftPadding() + width() / 2;

	return (origin + offset);
}

int PlotLineHandleH::centerPosToOrigin(int center)
{
	HorizHandlesArea *area = static_cast<HorizHandlesArea *>(parent());
	int offset = -area->leftPadding() + width() / 2;

	return (center - offset);
}

void PlotLineHandleH::mouseDoubleClickEvent(QMouseEvent *event)
{
	HorizHandlesArea *area = static_cast<HorizHandlesArea *>(parent());
	int center = (area->width() - area->leftPadding() - area->rightPadding()) / 2;
	setPosition(center);

	Q_EMIT reset();
}

PlotLineHandleV::PlotLineHandleV(const QPixmap &handleIcon, QWidget *parent,
			bool facingRight):
	PlotLineHandle(handleIcon, parent),
	m_facingRight(facingRight)
{
	m_innerSpacing = m_image.height() / 2;
	m_width = m_innerSpacing +  m_image.width()  + m_outerSpacing;
	m_height = m_image.height();
	setMinimumSize(m_width, m_height);
	setMaximumSize(m_width, m_height);
}

void PlotLineHandleV::triggerMove()
{
	Q_EMIT positionChanged(m_current_pos);
}

void PlotLineHandleV::setPosition(int pos)
{
	if (m_current_pos != pos)
		moveWithinParent(0, centerPosToOrigin(pos));
}

void PlotLineHandleV::setPositionSilenty(int pos)
{
	if (m_current_pos != pos)
		moveSilently(QPoint(0, centerPosToOrigin(pos)));
}

void PlotLineHandleV::updatePosition()
{
	moveSilently(QPoint(0, centerPosToOrigin(m_current_pos)));
}

void PlotLineHandleV::moveWithinParent(int x, int y)
{
	Q_UNUSED(x);

	VertHandlesArea *area = static_cast<VertHandlesArea *>(parent());

	int lower_limit = 0 + area->topPadding() - height() / 2;
	int upper_limit = area->height() - area->bottomPadding() - height() / 2 - 1;

	if (y < lower_limit)
		y = lower_limit;
	else if (y > upper_limit)
		y = upper_limit;

	int centerPos = originPosToCenter(y);
	int oldCenterPos = originPosToCenter(this->y());

	if (centerPos != oldCenterPos) {
		move(this->x(), y);
		if (!m_enable_silent_move)
			Q_EMIT positionChanged(centerPos);
	}
	m_enable_silent_move = false;
	m_current_pos = centerPos;
}

void PlotLineHandleV::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QPoint lineStart;
	QPoint imageTopLeft;

	p.setPen(m_pen);

	if (m_facingRight) {
		lineStart = QPoint(m_image.width(), m_image.height() / 2);
		imageTopLeft = QPoint(0, 0);
	} else {
		lineStart = QPoint(0, m_image.height() / 2);
		imageTopLeft = QPoint(m_innerSpacing, 0);
	}

	p.drawLine(lineStart, lineStart + QPoint(m_innerSpacing, 0));
	p.drawPixmap(imageTopLeft, m_image);
}

int PlotLineHandleV::originPosToCenter(int origin)
{
	VertHandlesArea *area = static_cast<VertHandlesArea *>(parent());
	int offset = -area->topPadding() + height() / 2;

	return (origin + offset);
}

int PlotLineHandleV::centerPosToOrigin(int center)
{
	VertHandlesArea *area = static_cast<VertHandlesArea *>(parent());
	int offset = -area->topPadding() + height() / 2;

	return (center - offset);
}

void PlotLineHandleV::mouseDoubleClickEvent(QMouseEvent *event)
{
	VertHandlesArea *area = static_cast<VertHandlesArea *>(parent());
	int center = (area->height() - area->topPadding() - area->bottomPadding()) / 2;
	setPosition(center);

	Q_EMIT reset();
}


FreePlotLineHandleH::FreePlotLineHandleH(const QPixmap &handleIcon,
			const QPixmap &beyondLeftIcon,
			const QPixmap &beyondRightIcon,
			QWidget *parent, bool facingRight):
		PlotLineHandleH(handleIcon, parent, facingRight),
		m_beyondLeftImage(beyondLeftIcon),
		m_beyondRightImage(beyondRightIcon),
		m_isBeyondLeft(false),
		m_isBeyondRight(false)
{
}

void FreePlotLineHandleH::moveWithinParent(int x, int y)
{
	Q_UNUSED(y);

	HorizHandlesArea *area = static_cast<HorizHandlesArea *>(parent());

	int lower_limit = 0 + area->leftPadding() - width() / 2;
	int upper_limit = area->width() - area->rightPadding() - width() / 2 - 1;

	int centerPos = originPosToCenter(x);
	int oldCenterPos = m_current_pos;

	if (centerPos != oldCenterPos) {
		m_isBeyondLeft = false;
		m_isBeyondRight = false;
		if (x < lower_limit) {
			x = lower_limit;
			m_isBeyondLeft = true;
		} else if (x > upper_limit) {
			x = upper_limit;
			m_isBeyondRight = true;
		}
		move(x, this->y());

		if (!m_enable_silent_move) {
			Q_EMIT positionChanged(centerPos);
			this->update();
		}
		m_current_pos = centerPos;
	}
	m_enable_silent_move = false;
}

void FreePlotLineHandleH::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QPoint lineStart;
	QPoint imageTopLeft;

	p.setPen(m_pen);

	if (m_facingBottom) {
		lineStart = QPoint(m_image.width() / 2,
				m_image.height());
		imageTopLeft = QPoint(0, 0);
	} else {
		lineStart = QPoint(m_image.width() / 2, 0);
		imageTopLeft = QPoint(0, m_innerSpacing);
	}

	if (m_isBeyondLeft) {
		p.drawPixmap(imageTopLeft, m_beyondLeftImage);
	} else if (m_isBeyondRight) {
		p.drawPixmap(imageTopLeft, m_beyondRightImage);
	} else {
		p.drawLine(lineStart, lineStart + QPoint(0, m_innerSpacing));
		p.drawPixmap(imageTopLeft, m_image);
	}
}

FreePlotLineHandleV::FreePlotLineHandleV(const QPixmap &handleIcon,
			const QPixmap &beyondTopIcon,
			const QPixmap &beyondBottomIcon,
			QWidget *parent, bool facingRight,
			const QString &name):
		PlotLineHandleV(handleIcon, parent, facingRight),
		m_beyondTopImage(beyondTopIcon),
		m_beyondBottomImage(beyondBottomIcon),
		m_isBeyondTop(false),
		m_isBeyondBottom(false),
		m_name(name)
{
}

void FreePlotLineHandleV::moveWithinParent(int x, int y)
{
	Q_UNUSED(x);

	VertHandlesArea *area = static_cast<VertHandlesArea *>(parent());

	int lower_limit = 0 + area->topPadding() - height() / 2;
	int upper_limit = area->height() - area->bottomPadding() - height() / 2 - 1;

	int centerPos = originPosToCenter(y);
	int oldCenterPos = m_current_pos;

	if (centerPos != oldCenterPos) {
		m_isBeyondTop = false;
		m_isBeyondBottom = false;
		if (y < lower_limit) {
			y = lower_limit;
			m_isBeyondTop = true;
		} else if (y > upper_limit) {
			y = upper_limit;
			m_isBeyondBottom = true;
		}
		move(this->x(), y);

		if (!m_enable_silent_move) {
			Q_EMIT positionChanged(centerPos);
			this->update();
		}
		m_current_pos = centerPos;
	}
	m_enable_silent_move = false;
}

void FreePlotLineHandleV::setName(const QString &name)
{
	if (m_name != name) {
		m_name = name;
		update();
	}
}

QString FreePlotLineHandleV::getName() const
{
	return m_name;
}

void FreePlotLineHandleV::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QPoint lineStart;
	QPoint imageTopLeft;

	p.setPen(m_pen);

	if (m_facingRight) {
		lineStart = QPoint(m_image.width(), m_image.height() / 2);
		imageTopLeft = QPoint(0, 0);
	} else {
		lineStart = QPoint(0, m_image.height() / 2);
		imageTopLeft = QPoint(m_innerSpacing, 0);
	}

	if (m_isBeyondTop) {
		p.drawPixmap(imageTopLeft, m_beyondTopImage);
	} else if (m_isBeyondBottom) {
		p.drawPixmap(imageTopLeft, m_beyondBottomImage);
	} else {
		p.drawLine(lineStart, lineStart + QPoint(m_innerSpacing, 0));

		if (!m_name.isEmpty()) {
			int fontSize = 10;

			const QString fontFamily = QApplication::font().family();

			while (QFontMetrics(QFont(fontFamily, fontSize)).horizontalAdvance(m_name)
					> m_image.width()) {
				fontSize--;
			}

			if (fontSize < 7) {
				// Set tool tip if size is to small as text
				// won't be visible
				setToolTip(m_name);
			} else {
				// disable tool tip if name changes
				// with something smaller
				setToolTip("");
			}

			const double textHeight = QFontMetrics(QFont(fontFamily, fontSize)).height();

			QPointF textPos(0.0, m_height / 2.0 + textHeight / 4.0);

			p.save();
			p.setPen(QPen(QBrush(Qt::white), 20));
			p.setFont(QFont(fontFamily, fontSize));
			p.drawText(textPos, m_name);
			p.restore();
		} else {
			p.drawPixmap(imageTopLeft, m_image);
		}
	}
}

RoundedHandleV::RoundedHandleV(const QPixmap &handleIcon,
			const QPixmap &beyondTopIcon,
			const QPixmap &beyondBottomIcon,
			QWidget *parent, bool facingRight,
			const QString &name, bool selectable):
		FreePlotLineHandleV(handleIcon, beyondTopIcon, beyondBottomIcon,
			 parent, facingRight, name)
{
	m_innerSpacing = m_image.height();
	m_width = m_innerSpacing +  m_image.width()  + m_outerSpacing;
	m_height = m_image.height();
	setMinimumSize(m_width, m_height);
	setMaximumSize(m_width, m_height);
	m_selected = false;
	m_selectable = selectable;
}

QColor RoundedHandleV::roundRectColor()
{
	return m_roundRectColor;
}

void RoundedHandleV::setRoundRectColor(const QColor &newColor)
{
	if (m_roundRectColor != newColor) {
		m_roundRectColor = newColor;
		this->update();
	}
}

void RoundedHandleV::setSelected(bool selected)
{
	if (m_selected != selected) {
		m_selected = selected;
		update();
	}
}

bool RoundedHandleV::isSelected() const
{
	return m_selected;
}

void RoundedHandleV::setSelectable(bool selectable)
{
	if (m_selectable != selectable) {
		m_selectable = selectable;
		update();
	}
}

void RoundedHandleV::paintEvent(QPaintEvent *pv)
{
	QPainter p(this);
	QRect rect(0, 0, m_image.width() - 1, m_image.height() - 1);

	if (m_selected && m_selectable) {
		p.setPen(QPen(Qt::white, 2, Qt::SolidLine));
	} else {
		p.setPen(QPen(m_roundRectColor, 1, Qt::SolidLine));
	}

	p.setBrush(m_roundRectColor);
	p.setRenderHint(QPainter::Antialiasing);
	p.drawRoundedRect(rect, 30, 30, Qt::RelativeSize);

	FreePlotLineHandleV::paintEvent(pv);
}

void RoundedHandleV::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (!m_selectable) {
		FreePlotLineHandleV::mouseDoubleClickEvent(event);
		return;
	}

	m_selected = !m_selected;
	update();

	Q_EMIT selected(m_selected);
}
