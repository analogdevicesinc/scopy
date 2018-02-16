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
		ApiObject *api, const QString& name,
		ToolLauncher *parent) :
	QWidget(static_cast<QWidget *>(parent)),
	ctx(ctx), run_button(runButton), api(api),
	name(name), saveOnExit(true)
{
        run_button->parentWidget()->setDisabled(false);

	connect(this, SIGNAL(detachedState(bool)), parent,
			SLOT(toolDetached(bool)));

	QSettings oldSettings;
	QFile tempFile(oldSettings.fileName() + ".bak");
	settings = new QSettings(tempFile.fileName(), QSettings::IniFormat);

	prefPanel = parent->getPrefPanel();
	connect(prefPanel, &Preferences::notify, this, &Tool::readPreferences);

	readPreferences();
}

Tool::~Tool()
{
	disconnect(prefPanel, &Preferences::notify, this, &Tool::readPreferences);

	run_button->parentWidget()->setDisabled(true);

	delete settings;
}


const QString &Tool::getName()
{
	return name;
}

void Tool::setName(const QString &name)
{
	this->name = name;
}

void Tool::settingsLoaded()
{

}

void Tool::readPreferences()
{
	saveOnExit = prefPanel->getSave_session_on_exit();
}

void Tool::attached()
{
	Q_EMIT detachedState(false);
}

void Tool::detached()
{
	Q_EMIT detachedState(true);
}


