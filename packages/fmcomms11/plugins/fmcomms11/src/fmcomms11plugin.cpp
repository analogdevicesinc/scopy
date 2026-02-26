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

#include "fmcomms11plugin.h"
#include "fmcomms11.h"

#include <QLoggingCategory>
#include <QLabel>
#include <deviceiconbuilder.h>
#include <style.h>
#include "scopy-fmcomms11_config.h"
#include <iioutil/connectionprovider.h>
#include <iio-widgets/iiowidgetgroup.h>

Q_LOGGING_CATEGORY(CAT_FMCOMMS11PLUGIN, "Fmcomms11Plugin")
using namespace scopy::fmcomms11;

bool Fmcomms11Plugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_FMCOMMS11PLUGIN) << "Check FMCOMMS11 compatibility";
	bool ret = false;
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_FMCOMMS11PLUGIN) << "No context available for FMCOMMS11";
		return false;
	}

	iio_device *adc = iio_context_find_device(conn->context(), "axi-ad9625-hpc");
	iio_device *dac = iio_context_find_device(conn->context(), "axi-ad9162-hpc");
	iio_device *attn = iio_context_find_device(conn->context(), "hmc1119");
	iio_device *vga = iio_context_find_device(conn->context(), "adl5240");
	ret = !!adc && !!dac && !!attn && !!vga;

	ConnectionProvider::close(m_param);

	return ret;
}

bool Fmcomms11Plugin::loadPage() { return false; }

bool Fmcomms11Plugin::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("FMCOMMS11");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();

	return true;
}

void Fmcomms11Plugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("fmcomms11Tool", "FMCOMMS11",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/gear_wheel.svg"));
}

void Fmcomms11Plugin::unload() {}

QString Fmcomms11Plugin::description() { return "This is a plugin for FMCOMMS11"; }

QString Fmcomms11Plugin::displayName() { return FMCOMMS11_PLUGIN_DISPLAY_NAME; }

bool Fmcomms11Plugin::onConnect()
{
	Connection *conn = ConnectionProvider::open(m_param);

	if(!conn) {
		qWarning(CAT_FMCOMMS11PLUGIN) << "No context available for FMCOMMS11";
		return false;
	}

	m_widgetGroup = new IIOWidgetGroup(this);

	FMCOMMS11 *tool = new FMCOMMS11(conn->context(), m_widgetGroup);
	m_toolList[0]->setTool(tool);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(false);

	return true;
}

bool Fmcomms11Plugin::onDisconnect()
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

void Fmcomms11Plugin::initMetadata()
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

#include "moc_fmcomms11plugin.cpp"
