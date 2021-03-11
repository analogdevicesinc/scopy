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
#include "tool_launcher_api.hpp"
#include "ui_tool_launcher.h"
#include "user_notes.hpp"
#include "spectrum_analyzer.hpp"
#include "apiobjectmanager.h"
#include "debugger.h"

#include <QJsonDocument>

namespace adiscope {
QString ToolLauncher_API::getIndexFile() const
{
	return tl->indexFile;
}

void ToolLauncher_API::setIndexFile(const QString &indexFile)
{
	tl->indexFile = indexFile;
}

bool ToolLauncher_API::menu_opened() const
{
	return tl->ui->btnMenu->isChecked();
}

void ToolLauncher_API::open_menu(bool open)
{
	tl->ui->btnMenu->setChecked(open);
}

bool ToolLauncher_API::hidden() const
{
	return !tl->isVisible();
}

void ToolLauncher_API::hide(bool hide)
{
	tl->setVisible(!hide);
}

void ToolLauncher_API::skip_calibration(bool skip)
{
	tl->skip_calibration = skip;
}

bool ToolLauncher_API::debugger_enabled()
{
	return tl->debugger_enabled;
}

void ToolLauncher_API::enable_debugger(bool enabled)
{
	tl->debugger_enabled = enabled;
	tl->prefPanel->setDebugger_enabled(enabled);
}

bool ToolLauncher_API::manual_calibration_enabled() const
{
	return tl->manual_calibration_enabled;
}

void ToolLauncher_API::enable_manual_calibration(bool enabled)
{
	tl->manual_calibration_enabled = enabled;
	tl->prefPanel->setManual_calib_enabled(enabled);
}

bool ToolLauncher_API::calibration_skipped()
{
	return tl->skip_calibration;
}

QList<QString> ToolLauncher_API::usb_uri_list()
{
	QList<QString> uri_list;
	auto list = tl->searchDevices();

	for (int i = 0; i < list.size(); i++) {
		uri_list.push_back(list[i]);
	}

	return uri_list;
}

bool ToolLauncher_API::connect(const QString& uri)
{
	QPushButton *btn = nullptr;
	bool did_connect = false;
	bool done = false;

	DeviceWidget* dev = nullptr;
	for (auto d : tl->devices) {
		if (d->uri() == uri) {
			dev = d;
			break;
		}
	}

	if (!dev) {
		btn = tl->addContext(uri);

		if (!btn) {
			return false;
		}
		for (auto d : tl->devices) {
			if (d->uri() == uri) {
				dev = d;
				break;
			}
		}
	}

	tl->connect(tl, &ToolLauncher::connectionDone,
			[&](bool success) {
		if (!success)
			done = true;
	});

	tl->connect(tl, &ToolLauncher::adcToolsCreated, [&]() {
		did_connect = true;
		done = true;
	});

	if (!dev) {
		return false;
	}

	tl->ui->btnConnect->click();
	dev->click();
	dev->connectButton()->click();

	do {
		QCoreApplication::processEvents();
		QThread::msleep(10);
	} while (!done);

	return did_connect;
}

void ToolLauncher_API::disconnect()
{
	tl->disconnect();
}

void ToolLauncher_API::addIp(const QString& ip)
{
	if (!ip.isEmpty()) {
		QtConcurrent::run(std::bind(&ToolLauncher::checkIp, tl, ip));
	}
}

void ToolLauncher_API::load(const QString& file)
{
	QSettings settings(file, QSettings::IniFormat);

	this->ApiObject::load(settings);

	if (tl->notesPanel)
		tl->notesPanel->api()->load(settings);
	if (tl->oscilloscope)
		tl->oscilloscope->getApi()->load(settings);
	if (tl->dmm)
		tl->dmm->getApi()->load(settings);
	if (tl->power_control)
		tl->power_control->getApi()->load(settings);
	if (tl->signal_generator)
		tl->signal_generator->getApi()->load(settings);
	if (tl->logic_analyzer)
		tl->logic_analyzer->getApi()->load(settings);
	if (tl->dio)
		tl->dio->getApi()->load(settings);
	if (tl->pattern_generator)
		tl->pattern_generator->getApi()->load(settings);
	if (tl->network_analyzer)
		tl->network_analyzer->getApi()->load(settings);
	if (tl->spectrum_analyzer)
		tl->spectrum_analyzer->getApi()->load(settings);

	ApiObjectManager::getInstance().load(settings);

	for (auto tool : tl->toolList)
		tool->settingsLoaded();
}

bool ToolLauncher_API::enableExtern(bool en)
{
	if (en && !tl->debugger_enabled) {
		return false;
	}
	tl->prefPanel->setExternal_script_enabled(en);
	return true;
}

bool ToolLauncher_API::enableCalibScript(bool en)
{
	if (en && !tl->manual_calibration_enabled) {
		return false;
	}
	tl->prefPanel->setManual_calib_script_enabled(en);
	return true;
}

bool ToolLauncher_API::reset()
{
	bool did_reconnect = false;

	/* Check if we are currently connected
	 * to any device */
	auto connectedDev = tl->getConnectedDevice();
	if (!connectedDev) {
		did_reconnect = true;
	}

	tl->resetSession();

	tl->connect(tl, &ToolLauncher::adcToolsCreated, [&]() {
		did_reconnect = true;
	});

	do {
		QCoreApplication::processEvents();
		QThread::msleep(10);
	} while (!did_reconnect);

	return did_reconnect;
}

void ToolLauncher_API::save(const QString& file)
{
	QSettings settings(file, QSettings::IniFormat);

	this->ApiObject::save(settings);
	this->tl->m_sessionInfo.save(settings);

	if (tl->notesPanel)
		tl->notesPanel->api()->save(settings);
	if (tl->oscilloscope)
		tl->oscilloscope->getApi()->save(settings);
	if (tl->dmm)
		tl->dmm->getApi()->save(settings);
	if (tl->power_control)
		tl->power_control->getApi()->save(settings);
	if (tl->signal_generator)
		tl->signal_generator->getApi()->save(settings);
	if (tl->logic_analyzer)
		tl->logic_analyzer->getApi()->save(settings);
	if (tl->dio)
		tl->dio->getApi()->save(settings);
	if (tl->pattern_generator)
		tl->pattern_generator->getApi()->save(settings);
	if (tl->network_analyzer)
		tl->network_analyzer->getApi()->save(settings);
	if (tl->spectrum_analyzer)
		tl->spectrum_analyzer->getApi()->save(settings);

	ApiObjectManager::getInstance().save(settings);
}

void ToolLauncher::addDebugWindow()
{
	DetachedWindow *window = new DetachedWindow();
	window->setWindowTitle("Debugger");
	Debugger *debug = new Debugger(ctx, filter,menu->getToolMenuItemFor(TOOL_DEBUGGER),
			&js_engine, this);
	QObject::connect(debug, &Debugger::newDebuggerInstance, this,
			 &ToolLauncher::addDebugWindow);

	window->setCentralWidget(debug);
	window->resize(sizeHint());
	window->show();
	debugWindows.append(window);
	debugInstances.append(debug);
}
}
