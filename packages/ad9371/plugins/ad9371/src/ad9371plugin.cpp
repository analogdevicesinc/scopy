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

#include "ad9371plugin.h"

#include <QLoggingCategory>
#include <QLabel>
#include <deviceiconbuilder.h>
#include <style.h>
#include "scopy-ad9371_config.h"
#include <iioutil/connectionprovider.h>
#include <iio-widgets/iiowidgetgroup.h>

#include "ad9371tool.h"
#include "ad9371advanced.h"

Q_LOGGING_CATEGORY(CAT_AD9371PLUGIN, "Ad9371Plugin")
using namespace scopy::ad9371;

bool Ad9371Plugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_AD9371PLUGIN) << "Check AD9371 compatibility";
	bool ret = false;
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_AD9371PLUGIN) << "No context available for AD9371";
		return false;
	}

	// Require ad9371-phy device
	iio_device *phyDev = iio_context_find_device(conn->context(), "ad9371-phy");
	if(phyDev) {
		// Exclude multi-chip setups (ad9371-phy-B present means ad9371x plugin should handle it)
		iio_device *phyDevB = iio_context_find_device(conn->context(), "ad9371-phy-B");
		ret = (phyDevB == nullptr);
	}

	ConnectionProvider::close(m_param);

	return ret;
}

bool Ad9371Plugin::loadPage() { return false; }

bool Ad9371Plugin::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("AD9371");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();

	return true;
}

void Ad9371Plugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("ad9371Tool", "AD9371",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("ad9371AdvancedTool", "AD9371 Advanced",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
}

void Ad9371Plugin::unload() {}

QString Ad9371Plugin::description() { return AD9371_PLUGIN_DESCRIPTION; }

QString Ad9371Plugin::displayName() { return AD9371_PLUGIN_DISPLAY_NAME; }

bool Ad9371Plugin::onConnect()
{
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_AD9371PLUGIN) << "No context available for AD9371";
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	// Detect DPD support for advanced tool
	bool has_dpd = false;
	iio_device *phyDev = iio_context_find_device(conn->context(), "ad9371-phy");
	if(phyDev) {
		iio_channel *tx0 = iio_device_find_channel(phyDev, "voltage0", true);
		has_dpd = tx0 && iio_channel_find_attr(tx0, "dpd_tracking_en");
	}

	AD9371 *ad9371 = new AD9371(conn->context(), m_widgetGroup);
	m_toolList[0]->setTool(ad9371);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(false);

	AD9371Advanced *advTool = new AD9371Advanced(conn->context(), m_widgetGroup, has_dpd);
	m_toolList[1]->setTool(advTool);
	m_toolList[1]->setEnabled(true);
	m_toolList[1]->setRunBtnVisible(false);

	return true;
}

bool Ad9371Plugin::onDisconnect()
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

	ConnectionProvider::close(m_param);
	return true;
}

void Ad9371Plugin::initMetadata()
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
