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
#include "dragzone.h"
#include "ui_dragzone.h"

using namespace adiscope;

DragZone::DragZone(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DragZone)
{
	ui->setupUi(this);
	position = 8;
	setAcceptDrops(true);
	this->installEventFilter(this);
}

DragZone::~DragZone()
{
	delete ui;
	this->removeEventFilter(this);
}

void DragZone::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("menu/option")){
		short from = (short)event->mimeData()->data("menu/option")[1];
		if (from == position){
			event->ignore();
			return;
		}
	}
	event->accept();
}

void DragZone::dragMoveEvent(QDragMoveEvent *event)
{
	Q_EMIT highlightLastSeparator(true);
	event->accept();
}

void DragZone::dragLeaveEvent(QDragLeaveEvent *event)
{
	Q_EMIT highlightLastSeparator(false);
	event->accept();
}

void DragZone::dropEvent(QDropEvent *event)
{
	Q_EMIT highlightLastSeparator(false);
	short from, to;
	if (event->source() == this && event->possibleActions() & Qt::MoveAction){
		return;
	}
	if (event->mimeData()->hasFormat("menu/option")){
		from = (short)event->mimeData()->data("menu/option")[1];
		to = (short)position;
		Q_EMIT requestPositionChange(from, to, true);
	}
}

int DragZone::getPosition() const
{
	return position;
}

void DragZone::setPosition(int value)
{
	position = value;
}

bool DragZone::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::DragEnter){
		QDragEnterEvent *enterEvent = static_cast<QDragEnterEvent *>(event);
		if (!enterEvent->mimeData()->hasFormat("menu/option"))
			return true;
		}
	return QWidget::event(event);
}
