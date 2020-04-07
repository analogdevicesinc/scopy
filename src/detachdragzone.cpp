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
#include "detachdragzone.h"
#include "tool_launcher.hpp"
#include <QMimeData>

#include "basemenuitem.h"

using namespace adiscope;

DetachDragZone::DetachDragZone(QWidget *parent) :
	ColoredQWidget(parent)
{
	ToolLauncher *tl = static_cast<ToolLauncher*>(parent->parent());
	if (tl->infoWidget != nullptr){
		connect(this, SIGNAL(changeText(QString)), tl->infoWidget,
			SLOT(setText(QString)));
	}

	setAcceptDrops(true);
	this->installEventFilter(this);
}

DetachDragZone::~DetachDragZone()
{
}

bool DetachDragZone::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::DragEnter){
		QDragEnterEvent *enterEvent = static_cast<QDragEnterEvent *>(event);
		if (!enterEvent->mimeData()->hasFormat(BaseMenuItem::menuItemMimeDataType))
			return true;
		}
	return QWidget::event(event);
}

void DetachDragZone::dragEnterEvent(QDragEnterEvent *event)
{
	if (!event->source()) {
		event->ignore();
		return;
	}

	Q_EMIT changeText(" Detach");
	event->accept();
}

void DetachDragZone::dragMoveEvent(QDragMoveEvent *event)
{
	event->accept();
}

void DetachDragZone::dragLeaveEvent(QDragLeaveEvent *event)
{
	Q_EMIT changeText(" Move");
	event->accept();
}

void DetachDragZone::dropEvent(QDropEvent *event)
{
	if (event->source() == this && event->possibleActions() & Qt::MoveAction){
		return;
	}
	if (event->mimeData()->hasFormat(BaseMenuItem::menuItemMimeDataType)){
		QObject *source = event->source();
		if (source) {
			ToolMenuItem *toolMenuItem = qobject_cast<ToolMenuItem *>(source);
			if (toolMenuItem) {
				toolMenuItem->detach();
			}
		}
	}
}
