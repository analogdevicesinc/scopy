/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "ad6676plugin.h"
#include "ad6676.h"
#include "ad6676_api.h"

#include <QLoggingCategory>
#include <QLabel>
#include <style.h>
#include "scopy-ad6676plugin_config.h"
#include <iioutil/connectionprovider.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <pluginbase/scopyjs.h>
#include <iio.h>

Q_LOGGING_CATEGORY(CAT_AD6676PLUGIN, "Ad6676Plugin")

using namespace scopy::ad6676;

bool Ad6676Plugin::compatible(QString param, QString category)
{
	qDebug(CAT_AD6676PLUGIN) << "Check AD6676 compatibility";

	Connection *conn = ConnectionProvider::open(param);
	if(!conn) {
		qWarning(CAT_AD6676PLUGIN) << "No context available for AD6676";
		return false;
	}

	// identify() logic from iio-oscilloscope: find "axi-ad6676-hpc"
	bool ret = iio_context_find_device(conn->context(), "axi-ad6676-hpc") != nullptr;

	ConnectionProvider::close(param);
	return ret;
}

bool Ad6676Plugin::loadPage() { return false; }

bool Ad6676Plugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void Ad6676Plugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("ad6676tool", "AD6676",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
}

void Ad6676Plugin::unload() {}

QString Ad6676Plugin::description() { return "AD6676 Wideband IF Receiver plugin ported from iio-oscilloscope"; }

QString Ad6676Plugin::displayName() { return AD6676PLUGIN_PLUGIN_DISPLAY_NAME; }

bool Ad6676Plugin::onConnect()
{
	Connection *conn = ConnectionProvider::open(m_param);
	if(!conn) {
		qWarning(CAT_AD6676PLUGIN) << "No context available for AD6676";
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	Ad6676 *tool = new Ad6676(conn->context(), m_widgetGroup);
	m_toolList[0]->setTool(tool);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(false);

	initApi();
	return true;
}

bool Ad6676Plugin::onDisconnect()
{
	if(m_api) {
		ScopyJS::GetInstance()->unregisterApi(m_api);
		delete m_api;
		m_api = nullptr;
	}

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

	ConnectionProvider::close(m_param);
	return true;
}

void Ad6676Plugin::initApi()
{
	m_api = new AD6676_API(this);
	m_api->setObjectName("ad6676");
	ScopyJS::GetInstance()->registerApi(m_api);
}

void Ad6676Plugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio"
	   ],
	   "exclude":["m2kplugin"]
	}
)plugin");
}
