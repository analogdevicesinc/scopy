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
#include "detachedwindowsmanager.h"

#include <QMimeData>


using namespace adiscope;

Tool::Tool(struct iio_context *ctx, ToolMenuItem *toolMenuItem,
		ApiObject *api, const QString& name,
		ToolLauncher *parent) :
	QWidget(static_cast<QWidget *>(parent)),
	ctx(ctx), run_button(toolMenuItem->getToolStopBtn()), api(api),
	name(name), saveOnExit(true), isDetached(false),
	window(nullptr), toolMenuItem(toolMenuItem)
{
	toolMenuItem->setDisabled(false);

	connect(this, &Tool::detachedState,
		parent, &ToolLauncher::toolDetached);
	connect(parent, &ToolLauncher::launcherClosed,
		this, &Tool::saveState);

	QSettings oldSettings;
	QFile tempFile(oldSettings.fileName() + ".bak");
	settings = new QSettings(tempFile.fileName(), QSettings::IniFormat);

	prefPanel = parent->getPrefPanel();
	connect(prefPanel, &Preferences::notify, this, &Tool::readPreferences);

	readPreferences();

	connect(api, &ApiObject::loadingFinished,
		this, &Tool::loadState);

	connect(toolMenuItem, &ToolMenuItem::detach,
		this, &Tool::detached);
	connect(this, &Tool::detachedState,
		toolMenuItem, &ToolMenuItem::setDetached);
}

Tool::~Tool()
{
	disconnect(prefPanel, &Preferences::notify, this, &Tool::readPreferences);

	toolMenuItem->setDisabled(true);

	delete settings;

	if (window) {
		// If the tool is in a DetachedWindow when it gets
		// destroyed make sure to close the window
		window->close();
	}
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

void Tool::saveState()
{
	if (!window) {
		settings->setValue(name + "/detached", isDetached);
		return;
	}

	settings->setValue(name + "/detached", isDetached);
	settings->setValue(name + "/geometry", window->saveGeometry());

	settings->sync();
}

void Tool::loadState()
{
	bool isDetached = settings->value(name + "/detached").toBool();
	if (isDetached) {
		detached();
		window->restoreGeometry(settings->value(name + "/geometry").toByteArray());
	}
}

void Tool::attached()
{
	Q_EMIT detachedState(false);
	isDetached = false;
	auto window = static_cast<DetachedWindow*>(this->window);
	disconnect(window, &DetachedWindow::closed,
		   this, &Tool::attached);
	DetachedWindowsManager::getInstance().returnWindow(window);
	this->window = nullptr;
}

void Tool::detached()
{
	if (isDetached) {
		// If it is already detached force it in the foreground
		static_cast<DetachedWindow*>(parent())->showWindow();
	} else {
		Q_EMIT detachedState(true);
		isDetached = true;
		auto window = DetachedWindowsManager::getInstance().getWindow();
		window->setCentralWidget(this);
		window->setWindowTitle("Scopy - " + name);
		window->show();
		connect(window, &DetachedWindow::closed,
			this, &Tool::attached);
		this->window = window;
	}
}


