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

#include "widgets/hoverwidget.h"

#include <QDebug>
#include <style.h>
#include <stylehelper.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_HOVERWIDGET, "HoverWidget")

using namespace scopy;

HoverWidget::HoverWidget(QWidget *content, QWidget *anchor, QWidget *parent)
	: QWidget(parent)
	, m_parent(parent)
	, m_anchor(anchor)
	, m_content(content)
	, m_anchorPos(HP_TOPLEFT)
	, m_contentPos(HP_TOPRIGHT)
	, is_dragging(false)
	, m_draggable(false)
	, m_relative(false)
	, m_relativeOffset(nullptr)
	, m_updateVisibility(false)
{
	Style::setBackgroundColor(this, QString("transparent"), true);
	m_lay = new QHBoxLayout(this);
	m_lay->setSizeConstraint(QLayout::SetFixedSize);
	m_lay->setContentsMargins(0, 0, 0, 0);
	setLayout(m_lay);

	if(content) {
		setContent(content);
	}
	if(anchor) {
		setAnchor(anchor);
	}
	if(parent) {
		setParent(parent);
	}

	hide();
}

void HoverWidget::setDraggable(bool draggable)
{
	m_draggable = draggable;
	StyleHelper::HoverWidget(this, m_draggable);
	if(!m_draggable) {
		updatePos();
	}
}

void HoverWidget::mousePressEvent(QMouseEvent *event)
{
	raise();
	if(event->button() == Qt::LeftButton && m_content->geometry().contains(event->pos()) && m_draggable) {
		is_dragging = true;
		mouse_pos = new QPoint(event->pos());
	}
}

void HoverWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton && is_dragging) {
		is_dragging = false;
		mouse_pos = new QPoint();
		updateRelativeOffset();
	}
}

void HoverWidget::mouseMoveEvent(QMouseEvent *event)
{
	if(!is_dragging) {
		return;
	}

	QPoint new_pos = event->pos() - *mouse_pos;
	QPoint topLeft = mapToParent(m_content->geometry().topLeft());
	QPoint new_topLeft = mapToParent(m_content->geometry().topLeft() + new_pos);
	QPoint bottomRight = mapToParent(m_content->geometry().bottomRight());
	QPoint new_bottomRight = mapToParent(m_content->geometry().bottomRight() + new_pos);

	// restrict horizontal movement
	if(new_topLeft.x() < 0) {
		new_pos.rx() = -topLeft.x();
	} else if(new_bottomRight.x() > m_parent->width()) {
		new_pos.rx() = m_parent->width() - bottomRight.x();
	}

	// restrict vertical movement
	if(new_topLeft.y() < 0) {
		new_pos.ry() = -topLeft.y();
	} else if(new_bottomRight.y() > m_parent->height()) {
		new_pos.ry() = m_parent->height() - bottomRight.y();
	}

	move(mapToParent(new_pos));
}

void HoverWidget::updateRelativeOffset()
{
	float height_diff = m_parent->height() - m_content->height();
	float width_diff = m_parent->width() - m_content->width();

	m_relativeOffset = new QPointF((width_diff - mapToParent(m_content->geometry().topLeft()).x()) / width_diff,
				       (height_diff - mapToParent(m_content->geometry().topLeft()).y()) / height_diff);
}

void HoverWidget::setRelative(bool relative) { m_relative = relative; }

void HoverWidget::setRelativeOffset(QPointF offset)
{
	m_relativeOffset = new QPointF(offset);
	moveToRelativePos();
}

void HoverWidget::setUpdateVisibility(bool update) { m_updateVisibility = update; }

void HoverWidget::setContent(QWidget *content)
{
	if(m_content) {
		m_lay->removeWidget(m_content);
		m_content->setParent(nullptr);
		m_content->removeEventFilter(this);
	}

	m_content = content;
	m_content->setParent(this);
	m_lay->addWidget(m_content);

	m_content->installEventFilter(this);
	updatePos();
}

void HoverWidget::setAnchor(QWidget *anchor)
{
	if(m_anchor) {
		m_anchor->removeEventFilter(this);
	}

	m_anchor = anchor;
	m_anchor->installEventFilter(this);
	m_relativeOffset = nullptr;
	updatePos();
}

void HoverWidget::setParent(QWidget *parent)
{
	if(m_parent) {
		m_parent->removeEventFilter(this);
	}

	bool visible = isVisible();
	m_parent = parent;
	QWidget::setParent(m_parent);
	m_parent->installEventFilter(this);
	updatePos();
	setVisible(visible);
}

HoverWidget::~HoverWidget() {}

bool HoverWidget::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == m_content) {
		if(event->type() == QEvent::HoverMove) {
			QMouseEvent *e = static_cast<QMouseEvent *>(event);
			if(m_content->geometry().contains(e->pos()) && m_draggable)
				m_content->setCursor(Qt::ClosedHandCursor);
			else
				m_content->setCursor(Qt::ArrowCursor);
		}
	}
	if(watched == m_anchor || watched == m_parent || watched == m_content) {
		if((event->type() == QEvent::Move || event->type() == QEvent::Resize) && (!m_draggable || m_relative)) {
			updatePos();
		}
	}
	if(m_updateVisibility && (watched == m_anchor || watched == m_parent)) {
		if(event->type() == QEvent::Hide)
			hide();
		if(event->type() == QEvent::Show)
			show();
	}

	return QObject::eventFilter(watched, event);
}

QPoint HoverWidget::getAnchorOffset() { return m_anchorOffset; }

void HoverWidget::setAnchorOffset(QPoint pt)
{
	m_anchorOffset = pt;
	m_relativeOffset = nullptr;
	updatePos();
}

HoverPosition HoverWidget::getAnchorPos() { return m_anchorPos; }

void HoverWidget::setAnchorPos(HoverPosition pos)
{
	m_anchorPos = pos;
	m_relativeOffset = nullptr;
	updatePos();
}

HoverPosition HoverWidget::getContentPos() { return m_contentPos; }

void HoverWidget::setContentPos(HoverPosition pos)
{
	m_contentPos = pos;
	m_relativeOffset = nullptr;
	updatePos();
}

void HoverWidget::updatePos()
{
	if(m_relative && m_relativeOffset) {
		moveToRelativePos();
	} else {
		moveToAnchor();
	}
}

void HoverWidget::moveToRelativePos()
{
	if(!m_content || !m_parent || !m_relativeOffset)
		return;

	int height_diff = m_parent->height() - m_content->height();
	int width_diff = m_parent->width() - m_content->width();

	QPoint new_pos = QPoint(width_diff - m_relativeOffset->x() * width_diff,
				height_diff - m_relativeOffset->y() * height_diff);
	move(new_pos);
}

void HoverWidget::moveToAnchor()
{
	if(!m_content || !m_anchor || !m_parent)
		return;

	QPoint global = m_anchor->mapToGlobal(QPoint(0, 0));
	QPoint mappedPoint = m_parent->mapFromGlobal(global);
	QPoint anchorPosition = QPoint(0, 0);
	QPoint contentPosition = QPoint(0, 0);

	switch(m_anchorPos) {
	case HP_LEFT:
		anchorPosition = QPoint(0, m_anchor->height() / 2);
		break;
	case HP_TOPLEFT:
		anchorPosition = QPoint(0, 0);
		break;
	case HP_TOP:
		anchorPosition = QPoint(m_anchor->width() / 2, 0);
		break;
	case HP_TOPRIGHT:
		anchorPosition = QPoint(m_anchor->width(), 0);
		break;
	case HP_RIGHT:
		anchorPosition = QPoint(m_anchor->width(), m_anchor->height() / 2);
		break;
	case HP_BOTTOMRIGHT:
		anchorPosition = QPoint(m_anchor->width(), m_anchor->height());
		break;
	case HP_BOTTOM:
		anchorPosition = QPoint(m_anchor->width() / 2, m_anchor->height());
		break;
	case HP_BOTTOMLEFT:
		anchorPosition = QPoint(0, m_anchor->height());
		break;
	case HP_CENTER:
		anchorPosition = QPoint(m_anchor->width() / 2, m_anchor->height() / 2);
		break;
	default:
		anchorPosition = QPoint(0, 0);
		break;
	}

	switch(m_contentPos) {
	case HP_LEFT:
		contentPosition = QPoint(-m_content->width(), -m_content->height() / 2);
		break;
	case HP_TOPLEFT:
		contentPosition = QPoint(-m_content->width(), -m_content->height());
		break;
	case HP_TOP:
		contentPosition = QPoint(-m_content->width() / 2, -m_content->height());
		break;
	case HP_TOPRIGHT:
		contentPosition = QPoint(0, -m_content->height());
		break;
	case HP_RIGHT:
		contentPosition = QPoint(0, -m_content->height() / 2);
		break;
	case HP_BOTTOMRIGHT:
		contentPosition = QPoint(0, 0);
		break;
	case HP_BOTTOM:
		contentPosition = QPoint(-m_content->width() / 2, 0);
		break;
	case HP_BOTTOMLEFT:
		contentPosition = QPoint(-m_content->width(), 0);
		break;
	case HP_CENTER:
		contentPosition = QPoint(-m_content->width() / 2, -m_content->height() / 2);
		break;
	default:
		contentPosition = QPoint(0, 0);
		break;
	}

	qDebug(CAT_HOVERWIDGET) << "moveAnchor"
				<< "mapped" << mappedPoint << "contentPosition" << contentPosition << "anchorPosition"
				<< anchorPosition << "offset" << m_anchorOffset;
	move(mappedPoint + contentPosition + anchorPosition + m_anchorOffset);
}

void HoverWidget::showEvent(QShowEvent *event)
{
	if(!m_draggable) {
		updatePos();
	}
	raise();
	QWidget::showEvent(event);
}

#include "moc_hoverwidget.cpp"
