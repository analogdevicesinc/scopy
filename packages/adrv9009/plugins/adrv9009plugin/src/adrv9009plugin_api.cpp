/*
 * Copyright (c) 2025 Analog Devices Inc.
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
 */

#include "adrv9009plugin_api.h"
#include "adrv9009plugin.h"
#include "adrv9009.h"
#include "adrv9009advanced.h"
#include <pluginbase/toolmenuentry.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_ADRV9009_API, "Adrv9009Plugin_API")

using namespace scopy::adrv9009;

Adrv9009Plugin_API::Adrv9009Plugin_API(Adrv9009Plugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

Adrv9009Plugin_API::~Adrv9009Plugin_API() {}

// --- Private helpers ---

QString Adrv9009Plugin_API::readFromWidget(const QString &key)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_ADRV9009_API) << "Widget group not available";
		return QString();
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_ADRV9009_API) << "Widget not found for key:" << key;
		return QString();
	}

	QPair<QString, QString> result = widget->read();
	return result.first;
}

void Adrv9009Plugin_API::writeToWidget(const QString &key, const QString &value)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_ADRV9009_API) << "Widget group not available";
		return;
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_ADRV9009_API) << "Widget not found for key:" << key;
		return;
	}

	widget->writeAsync(value);
}

// --- Tool management ---

QStringList Adrv9009Plugin_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// --- Generic widget access ---

QStringList Adrv9009Plugin_API::getWidgetKeys()
{
	if(!m_plugin->m_widgetGroup) {
		return QStringList();
	}
	return m_plugin->m_widgetGroup->keys();
}

QString Adrv9009Plugin_API::readWidget(const QString &key) { return readFromWidget(key); }

void Adrv9009Plugin_API::writeWidget(const QString &key, const QString &value) { writeToWidget(key, value); }

void Adrv9009Plugin_API::refresh()
{
	if(!m_plugin->m_widgetGroup) {
		return;
	}

	auto widgets = m_plugin->m_widgetGroup->getAll();
	for(auto *widget : widgets) {
		widget->readAsync();
	}
}

// --- Convenience: main tool ---

void Adrv9009Plugin_API::triggerRefresh()
{
	if(m_plugin->m_toolList.isEmpty()) {
		return;
	}
	Adrv9009 *tool = qobject_cast<Adrv9009 *>(m_plugin->m_toolList[0]->tool());
	if(tool && tool->m_refreshButton) {
		tool->m_refreshButton->click();
	}
}

void Adrv9009Plugin_API::triggerMcsSync()
{
	if(m_plugin->m_toolList.isEmpty()) {
		return;
	}
	Adrv9009 *tool = qobject_cast<Adrv9009 *>(m_plugin->m_toolList[0]->tool());
	if(tool && tool->m_mcsButton) {
		tool->m_mcsButton->click();
	}
}

// --- Convenience: advanced tool ---

void Adrv9009Plugin_API::triggerAdvancedRefresh()
{
	if(m_plugin->m_toolList.size() < 2) {
		return;
	}
	Adrv9009Advanced *tool = qobject_cast<Adrv9009Advanced *>(m_plugin->m_toolList[1]->tool());
	if(tool && tool->m_refreshButton) {
		tool->m_refreshButton->click();
	}
}

void Adrv9009Plugin_API::switchAdvancedTab(const QString &tabName)
{
	if(m_plugin->m_toolList.size() < 2) {
		return;
	}
	Adrv9009Advanced *tool = qobject_cast<Adrv9009Advanced *>(m_plugin->m_toolList[1]->tool());
	if(!tool) {
		return;
	}

	QPushButton *btn = nullptr;
	if(tabName == "CLK Settings")
		btn = tool->m_clkSettingsBtn;
	else if(tabName == "Calibrations")
		btn = tool->m_calibrationsBtn;
	else if(tabName == "TX Settings")
		btn = tool->m_txSettingsBtn;
	else if(tabName == "RX Settings")
		btn = tool->m_rxSettingsBtn;
	else if(tabName == "ORX Settings")
		btn = tool->m_orxSettingsBtn;
	else if(tabName == "FHM Setup")
		btn = tool->m_fhmSetupBtn;
	else if(tabName == "PA Protection")
		btn = tool->m_paProtectionBtn;
	else if(tabName == "GAIN Setup")
		btn = tool->m_gainSetupBtn;
	else if(tabName == "AGC Setup")
		btn = tool->m_agcSetupBtn;
	else if(tabName == "ARM GPIO")
		btn = tool->m_gpioConfigBtn;
	else if(tabName == "AUX DAC")
		btn = tool->m_auxDacBtn;
	else if(tabName == "JESD204 Settings")
		btn = tool->m_jesd204SettingsBtn;
	else if(tabName == "JESD Framer")
		btn = tool->m_jesdFramerBtn;
	else if(tabName == "JESD Deframer")
		btn = tool->m_jesdDeframerBtn;
	else if(tabName == "BIST")
		btn = tool->m_bistBtn;

	if(btn) {
		btn->click();
	} else {
		qWarning(CAT_ADRV9009_API) << "Unknown advanced tab name:" << tabName;
	}
}

#include "moc_adrv9009plugin_api.cpp"
