/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "dacplugin.h"
#include "dac_logging_categories.h"
#include "dacinstrument.h"
#include "dacutils.h"
#include "scopy-dac_config.h"

#include <QLabel>
#include <deviceiconbuilder.h>
#include <menusectionwidget.h>
#include <style.h>

#include <iioutil/connectionprovider.h>
#include <gui/deviceinfopage.h>
#include <pluginbase/preferences.h>

using namespace scopy;
using namespace scopy::dac;

bool DACPlugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_DAC) << "compatible";
	bool ret = false;
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(conn == nullptr)
		return ret;

	for(int i = 0; i < iio_context_get_devices_count(conn->context()); i++) {
		iio_device *dev = iio_context_get_device(conn->context(), i);
		for(int j = 0; j < iio_device_get_channels_count(dev); j++) {
			struct iio_channel *chn = iio_device_get_channel(dev, j);
			if(!iio_channel_is_output(chn)) {
				continue;
			}
			if(iio_channel_is_scan_element(chn) || DacUtils::checkDdsChannel(chn)) {
				ret = true;
				goto finish;
			}
		}
	}
finish:

	ConnectionProvider::GetInstance()->close(m_param);
	return ret;
}

bool DACPlugin::loadPage()
{
	m_page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_page);

	ConnectionProvider *c = ConnectionProvider::GetInstance();
	Connection *conn = c->open(m_param);
	auto deviceInfoPage = new DeviceInfoPage(conn);
	lay->addWidget(deviceInfoPage);
	lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));
	c->close(m_param);

	return true;
}

bool DACPlugin::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("DAC");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();
	return true;
}

void DACPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("dac", "DAC",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_signal_generator.svg"));
}

void DACPlugin::unload()
{
	if(m_page) {
		delete m_page;
	}
}

QString DACPlugin::description() { return "Tool for generic IIO DAC control."; }

QString DACPlugin::pkgName() { return DAC_PKG_NAME; }

QString DACPlugin::about()
{
	QString content = "DAC plugin";
	return content;
}

bool DACPlugin::onConnect()
{
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(conn == nullptr)
		return false;
	m_ctx = conn->context();
	dac = new DacInstrument(conn);
	m_toolList[0]->setTool(dac);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);
	connect(m_toolList.last(), &ToolMenuEntry::runToggled, dynamic_cast<DacInstrument *>(dac),
		&DacInstrument::runToggled);
	connect(dynamic_cast<DacInstrument *>(dac), &DacInstrument::running, m_toolList[0], &ToolMenuEntry::setRunning);
	return true;
}

bool DACPlugin::onDisconnect()
{
	qDebug(CAT_DAC) << "disconnect";
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
	if(m_ctx) {
		ConnectionProvider::GetInstance()->close(m_param);
	}
	return true;
}

void DACPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":10,
	   "category":[
		"iio",
		"dac"
	   ]
	}
)plugin");
}

QString DACPlugin::version() { return "0.1"; }

#include "moc_dacplugin.cpp"
