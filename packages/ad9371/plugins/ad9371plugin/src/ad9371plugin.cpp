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

#include "ad9371plugin.h"
#include "ad9371_api.h"

#include <QLoggingCategory>
#include <QLabel>
#include <iio.h>
#include <ad9371.h>
#include <ad9371advanced.h>
#include <style.h>
#include <gui/deviceiconbuilder.h>
#include <iio-widgets/iiowidgetgroup.h>

#include <iioutil/connectionprovider.h>
#include <pluginbase/scopyjs.h>

Q_LOGGING_CATEGORY(CAT_AD9371PLUGIN, "Ad9371Plugin")
using namespace scopy::ad9371;

bool Ad9371Plugin::compatible(QString m_param, QString category)
{
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(!conn) {
		qWarning(CAT_AD9371PLUGIN) << "Failed to open connection";
		return false;
	}

	// AD9371 requires ad9371-phy AND excludes ad9371-phy-B (dual-chip variant)
	iio_device *device = iio_context_find_device(conn->context(), "ad9371-phy");
	iio_device *deviceB = iio_context_find_device(conn->context(), "ad9371-phy-B");
	bool compatible = (device != nullptr) && (deviceB == nullptr);

	if(compatible) {
		qDebug(CAT_AD9371PLUGIN) << "AD9371 device found, plugin is compatible";
	} else {
		qDebug(CAT_AD9371PLUGIN) << "AD9371 device not found or dual-chip detected, plugin not compatible";
	}

	cp->close(m_param);
	return compatible;
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
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("ad9371tool", "AD9371",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("AD9371 Advanced", "AD9371 Advanced",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
}

void Ad9371Plugin::unload() {}

QString Ad9371Plugin::description() { return "AD9371 RF Transceiver control and configuration plugin"; }

bool Ad9371Plugin::onConnect()
{
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(!conn) {
		qWarning(CAT_AD9371PLUGIN) << "Failed to open connection for tool initialization";
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	// Create basic AD9371 tool with IIO context
	m_ad9371Tool = new Ad9371(conn->context(), m_widgetGroup);
	m_toolList[0]->setTool(m_ad9371Tool);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);

	// Create AD9371 Advanced tool
	iio_device *device = iio_context_find_device(conn->context(), "ad9371-phy");

	// Detect AD9375 and rename tools accordingly
	if(device && iio_device_find_debug_attr(device, "adi,dpd-model-version")) {
		m_toolList[0]->setName("ADRV9375");
		m_toolList[1]->setName("ADRV9375 Advanced");
	}

	if(device && iio_device_get_debug_attrs_count(device) > 0) {
		Ad9371Advanced *ad9371Advanced = new Ad9371Advanced(device, m_widgetGroup);
		m_toolList[1]->setTool(ad9371Advanced);
		m_toolList[1]->setEnabled(true);
		m_toolList[1]->setRunBtnVisible(true);
	}

	Q_EMIT toolListChanged();
	initApi();
	return true;
}

bool Ad9371Plugin::onDisconnect()
{
	if(m_api) {
		delete m_api;
		m_api = nullptr;
	}

	m_ad9371Tool = nullptr;

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

	qDebug(CAT_AD9371PLUGIN) << "AD9371 plugin disconnected successfully";
	return true;
}

void Ad9371Plugin::initApi()
{
	m_api = new Ad9371_API(this);
	m_api->setObjectName("ad9371");
	ScopyJS::GetInstance()->registerApi(m_api);
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
	   "exclude":[""]
	}
)plugin");
}
