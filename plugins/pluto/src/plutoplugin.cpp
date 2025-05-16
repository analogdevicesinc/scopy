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
 *
 */

#include "plutoplugin.h"

#include <QLoggingCategory>
#include <QLabel>
#include <deviceiconbuilder.h>
#include <style.h>

#include "ad936x.h"
#include "ad963xadvanced.h"

#include <iioutil/connectionprovider.h>

Q_LOGGING_CATEGORY(CAT_PLUTOPLUGIN, "PlutoPlugin")
using namespace scopy::pluto;

bool PlutoPlugin::compatible(QString m_param, QString category)
{

	qDebug(CAT_PLUTOPLUGIN) << "Check Pluto compatibility";
	bool ret = false;
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_PLUTOPLUGIN) << "No context available for Pluto";
		return false;
	}

	ret = false;
	int device_count = iio_context_get_devices_count(conn->context());
	for (int i = 0; i < device_count; ++i) {
	    iio_device *dev = iio_context_get_device(conn->context(), i);
	    const char *dev_name = iio_device_get_name(dev);
	    if (dev_name && QString(dev_name).contains("ad936", Qt::CaseInsensitive)) {
	        ret = true;
	        break;
	    }
	}

	ConnectionProvider::close(m_param);

	return ret;
}

bool PlutoPlugin::loadPage()
{
	return false;
}

bool PlutoPlugin::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("AD936X");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();

	return true;
}

void PlutoPlugin::loadToolList()
{
	m_toolList.append(
	        SCOPY_NEW_TOOLMENUENTRY("ad963xTool", "AD936X", ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/gear_wheel.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("ad963xAdvancedTool", "AD936X Advanced",
	                                          ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/gear_wheel.svg"));
}

void PlutoPlugin::unload()
{ }

QString PlutoPlugin::description() { return "This is a plugin for AD936X"; }

bool PlutoPlugin::onConnect()
{
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_PLUTOPLUGIN) << "No context available for Pluto";
		return false;
	}

	AD936X *ad936X = new AD936X(conn->context());
	m_toolList[0]->setTool(ad936X);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);

	AD936XAdvanced *ad936XAdvanced = new AD936XAdvanced(conn->context());
	m_toolList[1]->setTool(ad936XAdvanced);
	m_toolList[1]->setEnabled(true);
	m_toolList[1]->setRunBtnVisible(true);
	return true;
}

bool PlutoPlugin::onDisconnect()
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

	ConnectionProvider::close(m_param);
	return true;
}

void PlutoPlugin::initMetadata()
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
