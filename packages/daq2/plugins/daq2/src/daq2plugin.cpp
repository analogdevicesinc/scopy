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
 *
 */

#include "daq2plugin.h"
#include "daq2_api.h"
#include "daq2tool.h"
#include "scopy-daq2_config.h"

#include <QLabel>
#include <QLoggingCategory>
#include <deviceiconbuilder.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <iioutil/connectionprovider.h>
#include <pluginbase/scopyjs.h>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_DAQ2PLUGIN, "Daq2Plugin")
using namespace scopy::daq2;

bool Daq2Plugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_DAQ2PLUGIN) << "Check DAQ2 compatibility";
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_DAQ2PLUGIN) << "No context available for DAQ2";
		return false;
	}

	iio_context *ctx = conn->context();
	bool adcFound = iio_context_find_device(ctx, "axi-ad9680-hpc") != nullptr;
	bool dacFound = iio_context_find_device(ctx, "axi-ad9144-hpc") != nullptr;

	ConnectionProvider::close(m_param);

	return adcFound && dacFound;
}

bool Daq2Plugin::loadPage() { return false; }

bool Daq2Plugin::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("DAQ2");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();

	return true;
}

void Daq2Plugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("daq2Tool", "DAQ2",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
}

void Daq2Plugin::unload() {}

QString Daq2Plugin::description() { return "Plugin for DAQ2"; }

QString Daq2Plugin::displayName() { return DAQ2_PLUGIN_DISPLAY_NAME; }

bool Daq2Plugin::onConnect()
{
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_DAQ2PLUGIN) << "No context available for DAQ2";
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	DAQ2 *daq2 = new DAQ2(conn->context(), m_widgetGroup);
	m_toolList[0]->setTool(daq2);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(false);

	initApi();
	return true;
}

void Daq2Plugin::initApi()
{
	m_api = new Daq2_API(this);
	m_api->setObjectName("daq2");
	ScopyJS::GetInstance()->registerApi(m_api);
}

bool Daq2Plugin::onDisconnect()
{
	if(m_api) {
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

void Daq2Plugin::initMetadata()
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
