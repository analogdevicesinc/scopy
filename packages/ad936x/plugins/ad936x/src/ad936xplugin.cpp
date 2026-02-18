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

#include "ad936xplugin.h"
#include "ad936x_api.h"
#include "ad936x_advanced_api.h"
#include "fmcomms5_api.h"
#include "fmcomms5_advanced_api.h"

#include <QLoggingCategory>
#include <QLabel>
#include <deviceiconbuilder.h>
#include <style.h>
#include "scopy-ad936x_config.h"
#include <iioutil/connectionprovider.h>
#include <pluginbase/scopyjs.h>
#include <iio-widgets/iiowidgetgroup.h>

#include "ad936x/ad936x.h"
#include "ad936x/ad963xadvanced.h"

#include <fmcomms5/fmcomms5.h>
#include <fmcomms5/fmcomms5advanced.h>

Q_LOGGING_CATEGORY(CAT_AD936XPLUGIN, "Ad936xPlugin")
using namespace scopy::ad936x;

bool Ad936xPlugin::compatible(QString m_param, QString category)
{

	qDebug(CAT_AD936XPLUGIN) << "Check Pluto compatibility";
	bool ret = false;
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_AD936XPLUGIN) << "No context available for Pluto";
		return false;
	}

	ret = false;
	int device_count = iio_context_get_devices_count(conn->context());
	// this should work for any device from AD936x family
	for(int i = 0; i < device_count; ++i) {
		iio_device *dev = iio_context_get_device(conn->context(), i);
		const char *dev_name = iio_device_get_name(dev);
		if(dev_name && QString(dev_name).startsWith("ad936", Qt::CaseInsensitive)) {
			ret = true;
			break;
		}
	}

	ConnectionProvider::close(m_param);

	return ret;
}

bool Ad936xPlugin::loadPage() { return false; }

bool Ad936xPlugin::loadIcon()
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

void Ad936xPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("ad963xTool", "AD936X",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("ad963xAdvancedTool", "AD936X Advanced",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
}

void Ad936xPlugin::unload() {}

QString Ad936xPlugin::description() { return "This is a plugin for AD936X"; }

QString Ad936xPlugin::displayName() { return PLUTO_PLUGIN_DISPLAY_NAME; }

bool Ad936xPlugin::onConnect()
{
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_AD936XPLUGIN) << "No context available for Pluto";
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	// Check if FMCOMMS5 device is present (indicated by ad9361-phy-B device)
	m_isFmcomms5 = iio_context_find_device(conn->context(), "ad9361-phy-B") != nullptr;

	if(m_isFmcomms5) {
		FMCOMMS5 *fmcomms5 = new FMCOMMS5(conn->context(), m_widgetGroup);
		m_toolList[0]->setTool(fmcomms5);
		m_toolList[0]->setName("FMCOMMS5");
		m_toolList[0]->setEnabled(true);
		m_toolList[0]->setRunBtnVisible(false);

		Fmcomms5Advanced *fmcomms5Advanced = new Fmcomms5Advanced(conn->context(), m_widgetGroup);
		m_toolList[1]->setTool(fmcomms5Advanced);
		m_toolList[1]->setName("FMCOMMS5 Advanced");
		m_toolList[1]->setEnabled(true);
		m_toolList[1]->setRunBtnVisible(false);

	} else {
		AD936X *ad936X = new AD936X(conn->context(), m_widgetGroup);
		m_toolList[0]->setTool(ad936X);
		m_toolList[0]->setEnabled(true);
		m_toolList[0]->setRunBtnVisible(true);

		AD936XAdvanced *ad936XAdvanced = new AD936XAdvanced(conn->context(), m_widgetGroup);
		m_toolList[1]->setTool(ad936XAdvanced);
		m_toolList[1]->setEnabled(true);
		m_toolList[1]->setRunBtnVisible(true);
	}

	initApi();
	return true;
}

bool Ad936xPlugin::onDisconnect()
{
	if(m_api) {
		delete m_api;
		m_api = nullptr;
	}
	if(m_advancedApi) {
		delete m_advancedApi;
		m_advancedApi = nullptr;
	}
	if(m_fmcomms5Api) {
		delete m_fmcomms5Api;
		m_fmcomms5Api = nullptr;
	}
	if(m_fmcomms5AdvancedApi) {
		delete m_fmcomms5AdvancedApi;
		m_fmcomms5AdvancedApi = nullptr;
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

void Ad936xPlugin::initApi()
{
	m_api = new AD936X_API(this);
	m_api->setObjectName("ad936x");
	ScopyJS::GetInstance()->registerApi(m_api);

	if(m_isFmcomms5) {
		m_fmcomms5Api = new FMCOMMS5_API(this);
		m_fmcomms5Api->setObjectName("fmcomms5");
		ScopyJS::GetInstance()->registerApi(m_fmcomms5Api);

		m_fmcomms5AdvancedApi = new FMCOMMS5_ADVANCED_API(this);
		m_fmcomms5AdvancedApi->setObjectName("fmcomms5_advanced");
		ScopyJS::GetInstance()->registerApi(m_fmcomms5AdvancedApi);
	} else {
		m_advancedApi = new AD936X_ADVANCED_API(this);
		m_advancedApi->setObjectName("ad936x_advanced");
		ScopyJS::GetInstance()->registerApi(m_advancedApi);
	}
}

void Ad936xPlugin::initMetadata()
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
