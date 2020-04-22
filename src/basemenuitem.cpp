/*
 * Copyright 2019 Analog Devices, Inc.
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

#include "basemenuitem.h"

#include "utils.h"

#include "ui_basemenuitem.h"

#include <QDebug>
#include <QDrag>
#include <QMimeData>

using namespace adiscope;

const char* BaseMenuItem::menuItemMimeDataType = "menuItem";

BaseMenuItem::BaseMenuItem(QWidget* parent)
	: ColoredQWidget(parent)
	, d_ui(new Ui::BaseMenuItem)
	, d_position(0)
	, d_dragStartPosition(QPoint())
	, d_topDragBox(QRect())
	, d_centerDragBox(QRect())
	, d_botDragbox(QRect())
	, d_dragWidget(nullptr)
	, d_allowDrag(false)
{
	d_ui->setupUi(this);

	// Retain widget size when not visible
	Util::retainWidgetSizeWhenHidden(this);
	Util::retainWidgetSizeWhenHidden(d_ui->topSep);
	Util::retainWidgetSizeWhenHidden(d_ui->botSep);
	_enableBotSeparator(false);
	_enableTopSeparator(false);

	setAcceptDrops(true);

	installEventFilter(this);
}

BaseMenuItem::~BaseMenuItem()
{
	removeEventFilter(this);

	delete d_ui;
}

void BaseMenuItem::setWidget(QWidget* widget) { d_ui->contentsLayout->addWidget(widget); }

int BaseMenuItem::position() const { return d_position; }

void BaseMenuItem::setPosition(int position) { d_position = position; }

void BaseMenuItem::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		d_dragStartPosition = event->pos();
	}
}

void BaseMenuItem::mouseMoveEvent(QMouseEvent* event)
{
	if (!(event->buttons() & Qt::LeftButton)) {
		return;
	}

	if ((event->pos() - d_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
		return;
	}

	if (d_dragWidget && !d_allowDrag) {
		return;
	}

	QDrag* drag = new QDrag(this);
	QMimeData* mimeData = new QMimeData();

	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);
	dataStream << (short)d_position;
	mimeData->setData(menuItemMimeDataType, itemData);

	QPixmap pix;
	pix = grab().scaled(geometry().width(), geometry().height());

	// Hide this item while it is being dragged
	setVisible(false);

	drag->setPixmap(pix);
	drag->setMimeData(mimeData);

	Q_EMIT enableInfoWidget(true);

	drag->exec(Qt::MoveAction);

	Q_EMIT enableInfoWidget(false);

	// Make it visible again when the drag operation finishes
	setVisible(true);

	if (d_dragWidget) {
		d_allowDrag = false;
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

	d_topDragBox.setRect(0, 0, width, height / 2);
	d_botDragbox.setRect(0, height / 2, width, height / 2);

	if (!event->mimeData()->hasFormat(menuItemMimeDataType)) {
		event->ignore();
		return;
	}
	event->accept();
}

void BaseMenuItem::dragMoveEvent(QDragMoveEvent* event)
{
	if (event->answerRect().intersects(d_topDragBox)) {
		_enableBotSeparator(false);
		_enableTopSeparator(true);
		event->accept();
	} else if (event->answerRect().intersects(d_botDragbox)) {
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

	bool dropAfter = d_botDragbox.contains(event->pos());

	if (event->mimeData()->hasFormat(menuItemMimeDataType)) {
		short from = (short)event->mimeData()->data(menuItemMimeDataType)[1];
		short to = d_position;
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
	d_dragWidget = widget;
	d_dragWidget->installEventFilter(this);
}

bool BaseMenuItem::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::DragEnter) {
		QDragEnterEvent* enterEvent = static_cast<QDragEnterEvent*>(event);
		if (!enterEvent->mimeData()->hasFormat(menuItemMimeDataType))
			return true;
	}

	if (watched == d_dragWidget && event->type() == QEvent::MouseButtonPress) {
		d_allowDrag = true;
	}
	return QWidget::eventFilter(watched, event);
}

void BaseMenuItem::_enableBotSeparator(bool enable) { d_ui->botSep->setVisible(enable); }

void BaseMenuItem::_enableTopSeparator(bool enable) { d_ui->topSep->setVisible(enable); }
