/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "debuggerplugin.h"

#include "debugger/debuggerinstrument.h"
#include "iioexplorerinstrument.h"

#include <iio.h>

#include <QDebug>
#include <QElapsedTimer>
#include <QLabel>
#include <QLoggingCategory>
#include <QVBoxLayout>
#include <preferenceshelper.h>

#include <core/detachedtoolwindow.h>
#include <core/detachedtoolwindowmanager.h>
#include <iioutil/connectionprovider.h>
#include <pluginbase/preferences.h>
#include <gui/infopage.h>
#include <gui/deviceinfopage.h>

using namespace scopy;
using namespace scopy::debugger;

Q_LOGGING_CATEGORY(CAT_DEBUGGERPLUGIN, "DEBUGGERPLUGIN");
Q_LOGGING_CATEGORY(CAT_BENCHMARK, "Benchmark");

void DebuggerPlugin::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->init("debugger_v2_include_debugfs", true);
	p->init("plugins_use_debugger_v2", true);
}

bool DebuggerPlugin::compatible(QString m_param, QString category)
{
	bool ret = true;
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		return false;
	}
	ConnectionProvider::close(m_param);
	return ret;
}

bool DebuggerPlugin::loadPreferencesPage()
{
	Preferences *p = Preferences::GetInstance();

	m_preferencesPage = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(m_preferencesPage);

	MenuSectionCollapseWidget *generalSection = new MenuSectionCollapseWidget(
		"General", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET);
	generalSection->contentLayout()->setSpacing(10);
	layout->addWidget(generalSection);
	layout->setSpacing(0);
	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	auto use_debugger_v2 = PreferencesHelper::addPreferenceCheckBox(p, "plugins_use_debugger_v2",
									"Use Debugger V2 plugin", generalSection);
	auto debugger_include_debugfs = PreferencesHelper::addPreferenceCheckBox(
		p, "debugger_v2_include_debugfs", "Include debug attributes in IIO Explorer", generalSection);

	generalSection->contentLayout()->addWidget(use_debugger_v2);
	generalSection->contentLayout()->addWidget(debugger_include_debugfs);

	return true;
}

void DebuggerPlugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("debugger", "Debugger", ":/gui/icons/scopy-default/icons/gear_wheel.svg"));
	ToolMenuEntry::findToolMenuEntryById(m_toolList, "debugger")->setVisible(true);
}

void DebuggerPlugin::unload() {}

QString DebuggerPlugin::description() { return "IIO context explorer tool"; }

QString DebuggerPlugin::version() { return "0.1"; }

void DebuggerPlugin::saveSettings(QSettings &s)
{
	if(m_useDebuggerV2) {
		m_iioDebugger->saveSettings(s);
	}
}

void DebuggerPlugin::loadSettings(QSettings &s)
{
	if(m_useDebuggerV2) {
		m_iioDebugger->loadSettings(s);
	}
}

bool DebuggerPlugin::onConnect()
{
	m_conn = ConnectionProvider::open(m_param);
	if(!m_conn) {
		return false;
	}

	ToolMenuEntry *dbgTme = ToolMenuEntry::findToolMenuEntryById(m_toolList, "debugger");
	m_useDebuggerV2 = Preferences::get("plugins_use_debugger_v2").toBool();
	if(m_useDebuggerV2) {
		m_iioDebugger = new IIOExplorerInstrument(m_conn->context(), nullptr, nullptr);
		dbgTme->setTool(m_iioDebugger);
	} else {
		dbgTme->setTool(new DebuggerInstrument(m_conn->context(), nullptr, nullptr));
	}
	dbgTme->setEnabled(true);
	dbgTme->setRunBtnVisible(true);

	return true;
}

bool DebuggerPlugin::onDisconnect()
{
	for(ToolMenuEntry *tme : qAsConst(m_toolList)) {
		tme->setEnabled(false);
		tme->setRunBtnVisible(false);
		tme->setRunning(false);
		tme->tool()->deleteLater();
		tme->setTool(nullptr);
	}

	if(m_conn) {
		ConnectionProvider::close(m_param);
	}
	return true;
}

bool DebuggerPlugin::loadPage()
{
	m_page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_page);

	ConnectionProvider *c = ConnectionProvider::GetInstance();
	Connection *conn = c->open(m_param);
	auto deviceInfoPage = new DeviceInfoPage(conn);
	lay->addWidget(deviceInfoPage);
	lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));
	c->close(m_param);

	return true;
}

bool DebuggerPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void DebuggerPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":3,
	   "category":[
	      "iio"
	   ]
	}
)plugin");
}

#include "moc_debuggerplugin.cpp"
