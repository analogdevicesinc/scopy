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

#include "cn0357.h"

#include <QLoggingCategory>
#include <iio.h>
#include <iioutil/connectionprovider.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <style.h>

#include "cn0357tool.h"

Q_LOGGING_CATEGORY(CAT_CN0357PLUGIN, "Cn0357Plugin")
using namespace scopy::cn0357;

bool Cn0357Plugin::compatible(QString param, QString category)
{
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(param);
	if(!conn)
		return false;

	bool ret = (iio_context_find_device(conn->context(), "ad7790") != nullptr);

	cp->close(param);
	return ret;
}

bool Cn0357Plugin::loadPage() { return false; }

bool Cn0357Plugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void Cn0357Plugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("cn0357tool", "CN0357",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
}

void Cn0357Plugin::unload() {}

QString Cn0357Plugin::description() { return "CN0357 Electrochemical Gas Concentration Sensor"; }

bool Cn0357Plugin::onConnect()
{
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(!conn) {
		qWarning(CAT_CN0357PLUGIN) << "Failed to open connection";
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	Cn0357Tool *tool = new Cn0357Tool(conn->context(), m_widgetGroup);
	m_toolList[0]->setTool(tool);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(false);

	return true;
}

bool Cn0357Plugin::onDisconnect()
{
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunning(false);
		tool->setRunBtnVisible(false);
		QWidget *w = tool->tool();
		if(w) {
			tool->setTool(nullptr);
			delete(w);
		}
	}

	if(m_widgetGroup) {
		delete m_widgetGroup;
		m_widgetGroup = nullptr;
	}

	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	cp->close(m_param);
	return true;
}

void Cn0357Plugin::initMetadata()
{
	loadMetadata(R"plugin(
	{
	   "priority":2,
	   "category":["iio"],
	   "exclude":[""]
	}
)plugin");
}
