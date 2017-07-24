/*
 * Copyright 2017 Analog Devices, Inc.
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

#include "tool.hpp"
#include "tool_launcher.hpp"

#include <QMimeData>


using namespace adiscope;

Tool::Tool(struct iio_context *ctx, QPushButton *runButton,
		ApiObject *api, ToolLauncher *parent) :
	QWidget(static_cast<QWidget *>(parent)),
	ctx(ctx), run_button(runButton), api(api)
{
	run_button->parentWidget()->setDisabled(false);

	connect(this, SIGNAL(detachedState(bool)), parent,
			SLOT(toolDetached(bool)));

	QSettings oldSettings;
	QFile tempFile(oldSettings.fileName() + ".bak");
	settings = new QSettings(tempFile.fileName(), QSettings::IniFormat);

	setAcceptDrops(true);

	connect(this, SIGNAL(changeText(QString)), parent->infoWidget,
		SLOT(setText(QString)));
	connect(this, SIGNAL(detachTool(int)), parent,
					SLOT(detachToolOnPosition(int)));
}

Tool::~Tool()
{
	run_button->parentWidget()->setDisabled(true);

	delete settings;
}

void Tool::attached()
{
	Q_EMIT detachedState(false);
}

void Tool::detached()
{
	Q_EMIT detachedState(true);
}

void Tool::dragEnterEvent(QDragEnterEvent *event)
{
	Q_EMIT changeText(" Detach");
	event->accept();
}

void Tool::dragMoveEvent(QDragMoveEvent *event)
{
	event->accept();
}

void Tool::dragLeaveEvent(QDragLeaveEvent *event)
{
	Q_EMIT changeText(" Move");
	event->accept();
}

void Tool::dropEvent(QDropEvent *event)
{
	short position;
	if (event->source() == this && event->possibleActions() & Qt::MoveAction){
		return;
	}
	if (event->mimeData()->hasFormat("menu/option")){
		position = (short)event->mimeData()->data("menu/option")[1];
		Q_EMIT detachTool(position);
	}
}
