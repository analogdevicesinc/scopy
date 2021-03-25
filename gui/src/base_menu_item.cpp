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

#include "utils.hpp"

#include "ui_base_menu_item.h"

#include <QDebug>
#include <QDrag>
#include <QMimeData>

#include <scopy/gui/base_menu.hpp>
#include <scopy/gui/base_menu_item.hpp>

using namespace scopy::gui;

const char* BaseMenuItem::menuItemMimeDataType = "menuItem";

BaseMenuItem::BaseMenuItem(QWidget* parent)
	: ColoredQWidget(parent)
	, m_ui(new Ui::BaseMenuItem)
	, m_menu(nullptr)
	, m_position(0)
	, m_dragStartPosition(QPoint())
	, m_topDragBox(QRect())
	, m_centerDragBox(QRect())
	, m_botDragbox(QRect())
	, m_dragWidget(nullptr)
	, m_allowDrag(false)
{
	m_ui->setupUi(this);

	// Retain widget size when not visible
	Util::retainWidgetSizeWhenHidden(this);
	Util::retainWidgetSizeWhenHidden(m_ui->lineTopSep);
	Util::retainWidgetSizeWhenHidden(m_ui->lineBotSep);
	_enableBotSeparator(false);
	_enableTopSeparator(false);

	setAcceptDrops(true);

	installEventFilter(this);
}

BaseMenuItem::~BaseMenuItem()
{
	removeEventFilter(this);

	delete m_ui;
}

void BaseMenuItem::setWidget(QWidget* widget) { m_ui->contentsLayout->addWidget(widget); }

int BaseMenuItem::position() const { return m_position; }

void BaseMenuItem::setPosition(int position) { m_position = position; }

void BaseMenuItem::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		m_dragStartPosition = event->pos();
	}
}

void BaseMenuItem::mouseMoveEvent(QMouseEvent* event)
{
	if (!(event->buttons() & Qt::LeftButton)) {
		return;
	}

	if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
		return;
	}

	if (m_dragWidget && !m_allowDrag) {
		return;
	}

	QDrag* drag = new QDrag(this);
	QMimeData* mimeData = new QMimeData();

	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);
	dataStream << (short)m_position;
	mimeData->setData(menuItemMimeDataType, itemData);

	QPixmap pix;
	pix = grab().scaled(geometry().width(), geometry().height());

	// Hide this item while it is being dragged
	setVisible(false);

	drag->setPixmap(pix);
	drag->setMimeData(mimeData);
	drag->setHotSpot(event->pos());

	Q_EMIT enableInfoWidget(true);

	drag->exec(Qt::MoveAction);

	Q_EMIT enableInfoWidget(false);

	// Make it visible again when the drag operation finishes
	setVisible(true);

	if (m_dragWidget) {
		m_allowDrag = false;
	}
}

void BaseMenuItem::dragLeaveEvent(QDragLeaveEvent* event)
{
	_enableBotSeparator(false);
	_enableTopSeparator(false);
	event->accept();
}

void BaseMenuItem::dragEnterEvent(QDragEnterEvent* event)
{
	if (!event->source()) {
		event->ignore();
		return;
	}

	int width = this->geometry().width();
	int height = this->geometry().height();

	m_topDragBox.setRect(0, 0, width, height / 2);
	m_botDragbox.setRect(0, height / 2, width, height / 2);

	if (!event->mimeData()->hasFormat(menuItemMimeDataType)) {
		event->ignore();
		return;
	}
	event->accept();
}

void BaseMenuItem::dragMoveEvent(QDragMoveEvent* event)
{
	if (event->answerRect().intersects(m_topDragBox)) {
		_enableBotSeparator(false);
		_enableTopSeparator(true);
		event->accept();
	} else if (event->answerRect().intersects(m_botDragbox)) {
		_enableBotSeparator(true);
		_enableTopSeparator(false);
		event->accept();
	} else {
		_enableBotSeparator(false);
		_enableTopSeparator(false);
		event->ignore();
	}
}

void BaseMenuItem::dropEvent(QDropEvent* event)
{
	_enableBotSeparator(false);
	_enableTopSeparator(false);

	if (!event->source()) {
		return;
	}

	if (event->source() == this && event->possibleActions() & Qt::MoveAction) {
		return;
	}

	bool dropAfter = m_botDragbox.contains(event->pos());

	if (event->mimeData()->hasFormat(menuItemMimeDataType)) {
		short from = (short)event->mimeData()->data(menuItemMimeDataType)[1];
		short to = m_position;
		if (dropAfter)
			to++;
		Q_EMIT moveItem(from, to);
	}
}

void BaseMenuItem::enterEvent(QEvent* event) { event->accept(); }

void BaseMenuItem::leaveEvent(QEvent* event) { event->accept(); }

void BaseMenuItem::setDragWidget(QWidget* widget)
{
	// Set the widget that must be used for dragging
	m_dragWidget = widget;
	m_dragWidget->installEventFilter(this);
}

bool BaseMenuItem::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::DragEnter) {
		QDragEnterEvent* enterEvent = static_cast<QDragEnterEvent*>(event);
		if (!enterEvent->mimeData()->hasFormat(menuItemMimeDataType))
			return true;
	}

	if (watched == m_dragWidget && event->type() == QEvent::MouseButtonPress) {
		m_allowDrag = true;
	}
	return QWidget::eventFilter(watched, event);
}

BaseMenu* BaseMenuItem::getOwner() const { return m_menu; }

void BaseMenuItem::setOwner(BaseMenu* menu) { m_menu = menu; }

void BaseMenuItem::_enableBotSeparator(bool enable) { m_ui->lineBotSep->setVisible(enable); }

void BaseMenuItem::_enableTopSeparator(bool enable) { m_ui->lineTopSep->setVisible(enable); }
