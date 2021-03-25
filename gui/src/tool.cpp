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

#include "tool.hpp"

#include "detached_windows_manager.hpp"

#include <QMimeData>

#include <scopy/gui/tool_launcher.hpp>

using namespace scopy::gui;

Tool::Tool(struct iio_context* ctx, ToolMenuItem* toolMenuItem, scopy::core::ApiObject* api, const QString& name,
	   ToolLauncher* parent)
	: QWidget(static_cast<QWidget*>(parent))
	, m_ctx(ctx)
	, m_run_button(toolMenuItem->getToolStopBtn())
	, m_api(api)
	, m_name(name)
	, m_saveOnExit(true)
	, m_isDetached(false)
	, m_running(false)
	, m_window(nullptr)
	, m_toolMenuItem(toolMenuItem)
{
	toolMenuItem->setDisabled(false);

	QSettings oldSettings;
	QFile tempFile(oldSettings.fileName() + ".bak");
	m_settings = new QSettings(tempFile.fileName(), QSettings::IniFormat);

	readPreferences();

	if (api) {
		connect(api, &scopy::core::ApiObject::loadingFinished, this, &Tool::loadState);
	}

	connect(toolMenuItem, &ToolMenuItem::detach, this, &Tool::detached);
	connect(this, &Tool::detachedState, toolMenuItem, &ToolMenuItem::setDetached);
}

Tool::~Tool()
{
	m_run_button->setChecked(false);
	m_toolMenuItem->setDisabled(true);

	delete m_settings;

	if (m_window) {
		// If the tool is in a DetachedWindow when it gets
		// destroyed make sure to close the window
		m_window->close();
	}
}

const QString& Tool::getName() { return m_name; }

void Tool::setName(const QString& name) { this->m_name = name; }

void Tool::settingsLoaded() {}

/* Tools that use file dialogs should overload this method
to ensure their file dialogs are configured correspondingly */
void Tool::setNativeDialogs(bool nativeDialogs) { m_useNativeDialogs = nativeDialogs; }

scopy::core::ApiObject* Tool::getApi() { return m_api; }

void Tool::readPreferences()
{
	//	saveOnExit = prefPanel->getSave_session_on_exit();
}

void Tool::saveState()
{
	if (!m_window) {
		m_settings->setValue(m_name + "/detached", m_isDetached);
		return;
	}

	m_settings->setValue(m_name + "/detached", m_isDetached);
	m_settings->setValue(m_name + "/geometry", m_window->saveGeometry());

	m_settings->sync();
}

void Tool::loadState()
{
	bool isDetached = m_settings->value(m_name + "/detached").toBool();
	if (isDetached) {
		detached();
		m_window->restoreGeometry(m_settings->value(m_name + "/geometry").toByteArray());
	}
}

void Tool::attached()
{
	Q_EMIT detachedState(false);
	m_isDetached = false;
	auto window = static_cast<DetachedWindow*>(this->m_window);
	disconnect(window, &DetachedWindow::closed, this, &Tool::attached);
	DetachedWindowsManager::getInstance().returnWindow(window);
	this->m_window = nullptr;
}

void Tool::detached()
{
	if (m_isDetached) {
		// If it is already detached force it in the foreground
		static_cast<DetachedWindow*>(parent())->showWindow();
	} else {
		Q_EMIT detachedState(true);
		m_isDetached = true;
		auto window = DetachedWindowsManager::getInstance().getWindow();
		window->setCentralWidget(this);
		window->setWindowTitle("Scopy - " + m_name);
		window->show();
		connect(window, &DetachedWindow::closed, this, &Tool::attached);
		this->m_window = window;
	}
}

void Tool::run() {}
void Tool::stop() {}
void Tool::single() {}
bool Tool::isRunning() { return m_running; }
