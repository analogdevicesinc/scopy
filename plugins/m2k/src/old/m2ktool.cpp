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

#include "m2ktool.hpp"
#include <QMimeData>


using namespace adiscope;
using namespace adiscope::m2k;

M2kTool::M2kTool(struct iio_context *ctx, ToolMenuEntry *tme,
		ApiObject *api, const QString& name,
		QWidget *parent) :
	QWidget(parent),
	ctx(ctx), api(api),
	name(name), saveOnExit(true), isDetached(false), m_running(false),
	window(nullptr), tme(tme)
{
	tme->setEnabled(true);

//	connect(this, &Tool::detachedState,
//		parent, &ToolLauncher::toolDetached);
//	connect(parent, &ToolLauncher::launcherClosed,
//		this, &Tool::saveState);

	p = Preferences::GetInstance();
	connect(p, &Preferences::preferenceChanged, this, &M2kTool::readPreferences);
	M2kTool::readPreferences();
//#ifndef __ANDROID__
//	if (api) {
//		connect(api, &ApiObject::loadingFinished,
//			this, &Tool::loadState);
//	}

//	connect(toolMenuItem, &ToolMenuItem::detach,
//		this, &Tool::detached);
//#endif
//	connect(this, &Tool::detachedState,
//		toolMenuItem, &ToolMenuItem::setDetached);


}

M2kTool::~M2kTool()
{
	disconnect(p, &Preferences::preferenceChanged, this, &M2kTool::readPreferences);

	tme->setRunning(false);
	tme->setEnabled(false);


}

const QString &M2kTool::getName()
{
	return name;
}

void M2kTool::setName(const QString &name)
{
	this->name = name;
}

void M2kTool::settingsLoaded()
{

}

/* Tools that use file dialogs should overload this method
to ensure their file dialogs are configured correspondingly */
void M2kTool::setNativeDialogs(bool nativeDialogs)
{
	m_useNativeDialogs = nativeDialogs;
}

ApiObject *M2kTool::getApi()
{
	return api;
}

void M2kTool::readPreferences()
{
	saveOnExit = p->get("general_save_on_exit").toBool();
	m_useNativeDialogs = p->get("general_native_dialogs").toBool();;
}

void M2kTool::saveState()
{
//	if (!window) {
//		settings->setValue(name + "/detached", isDetached);
//		return;
//	}

//	settings->setValue(name + "/detached", isDetached);
//	settings->setValue(name + "/geometry", window->saveGeometry());

//	settings->sync();
}


#ifndef __ANDROID__
void M2kTool::loadState()
{
//	bool isDetached = settings->value(name + "/detached").toBool();
//	if (isDetached) {
//		detached();
//		window->restoreGeometry(settings->value(name + "/geometry").toByteArray());
//	}
}

void M2kTool::attached()
{
//	Q_EMIT detachedState(false);
//	isDetached = false;
//	auto window = static_cast<DetachedWindow*>(this->window);
//	disconnect(window, &DetachedWindow::closed,
//		   this, &Tool::attached);
//	DetachedWindowsManager::getInstance().returnWindow(window);
//	this->window = nullptr;
}

void M2kTool::detached()
{
//	if (isDetached) {
//		// If it is already detached force it in the foreground
//		static_cast<DetachedWindow*>(parent())->showWindow();
//	} else {
//		Q_EMIT detachedState(true);
//		isDetached = true;
//		auto window = DetachedWindowsManager::getInstance().getWindow();
//		window->setCentralWidget(this);
//		window->setWindowTitle("Scopy - " + name);
//		window->show();
//		connect(window, &DetachedWindow::closed,
//			this, &Tool::attached);
//		this->window = window;
//	}
}
#endif

void M2kTool::run()
{

}
void M2kTool::stop()
{

}
void M2kTool::single()
{

}
bool M2kTool::isRunning()
{
	return m_running;
}



