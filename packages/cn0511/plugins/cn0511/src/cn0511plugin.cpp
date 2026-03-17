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

#include "cn0511plugin.h"
#include "cn0511.h"

#include <QLoggingCategory>
#include <QLabel>
#include <deviceiconbuilder.h>
#include <style.h>
#include "scopy-cn0511_config.h"
#include <iioutil/connectionprovider.h>
#include <iio-widgets/iiowidgetgroup.h>

Q_LOGGING_CATEGORY(CAT_CN0511PLUGIN, "Cn0511Plugin")
using namespace scopy::cn0511;

bool Cn0511Plugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_CN0511PLUGIN) << "Check CN0511 compatibility";
	bool ret = false;
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_CN0511PLUGIN) << "No context available for CN0511";
		return false;
	}

	iio_context *ctx = conn->context();
	ret = (iio_context_find_device(ctx, "ad9166") != nullptr) &&
		(iio_context_find_device(ctx, "ad9166-amp") != nullptr);

	ConnectionProvider::close(m_param);

	return ret;
}

bool Cn0511Plugin::loadPage() { return false; }

bool Cn0511Plugin::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("CN0511");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();

	return true;
}

void Cn0511Plugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("cn0511Tool", "CN0511",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
}

void Cn0511Plugin::unload() {}

QString Cn0511Plugin::description() { return CN0511_PLUGIN_DESCRIPTION; }

QString Cn0511Plugin::displayName() { return CN0511_PLUGIN_DISPLAY_NAME; }

bool Cn0511Plugin::onConnect()
{
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_CN0511PLUGIN) << "No context available for CN0511";
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	iio_context *ctx = conn->context();

	// Initialize device settings as per original cn0511.c
	iio_device *dac = iio_context_find_device(ctx, "ad9166");
	if(dac) {
		int ret = iio_device_attr_write_longlong(dac, "fir85_enable", 1);
		if(ret < 0) {
			qWarning(CAT_CN0511PLUGIN) << "Failed to enable FIR85. Error:" << ret;
		}

		ret = iio_device_attr_write_longlong(dac, "sampling_frequency", 6000000000LL);
		if(ret < 0) {
			qWarning(CAT_CN0511PLUGIN) << "Failed to set sampling frequency. Error:" << ret;
		}
	}

	CN0511 *cn0511 = new CN0511(ctx, m_widgetGroup);
	m_toolList[0]->setTool(cn0511);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(false);

	return true;
}

bool Cn0511Plugin::onDisconnect()
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

void Cn0511Plugin::initMetadata()
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
