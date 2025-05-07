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

#include "jesdstatusplugin.h"
#include <iioutil/connectionprovider.h>

#include <QLoggingCategory>
#include <QLabel>

#include <style.h>
#include "jesdstatus.h"

Q_LOGGING_CATEGORY(CAT_JESDSTATUSPLUGIN, "JesdStatusPlugin")
using namespace scopy::jesdstatus;

bool JesdStatusPlugin::compatible(QString m_param, QString category)
{
	bool ret = false;
	auto &&cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(!conn) {
		qWarning() << "The context is not compatible with the JesdStatus Plugin!";
		return ret;
	}

	auto lst = scanCompatibleDevices(conn->context());
	ret = lst.size();

	cp->close(m_param);
	return ret;
}

QList<QString> JesdStatusPlugin::scanCompatibleDevices(struct iio_context *ctx)
{
	QList<QString> devList = {};
	unsigned int devs = iio_context_get_devices_count(ctx);
	for(unsigned int i = 0; i < devs; i++) {
		struct iio_device *dev = iio_context_get_device(ctx, i);
		auto status = iio_device_find_attr(dev, "status");
		if(!status) {
			continue;
		}
		QString name = iio_device_get_name(dev);
		QString id = iio_device_get_id(dev);
		QString label = iio_device_get_label(dev);
		if(name.contains("jesd") || id.contains("jesd") || label.contains("jesd")) {
			devList.push_back(id);
		}
	}
	return devList;
}

bool JesdStatusPlugin::loadPage() { return false; }

bool JesdStatusPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void JesdStatusPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("jesdstatus", "Jesd Status",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_voltmeter.svg"));
}

void JesdStatusPlugin::unload()
{ /*delete m_infoPage;*/
}

QString JesdStatusPlugin::description() { return "Tool for exposing JESD status in compatible devices"; }

bool JesdStatusPlugin::onConnect()
{
	auto &&cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(!conn) {
		return false;
	}
	QList<struct iio_device *> devLst;
	auto lst = scanCompatibleDevices(conn->context());
	for(auto id : lst) {
		auto dev = iio_context_find_device(conn->context(), id.toUtf8());
		if(dev) {
			devLst.push_back(dev);
		}
	}

	JesdStatus *jesdStatus = new JesdStatus(devLst);
	m_toolList[0]->setTool(jesdStatus);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);
	return true;
}

bool JesdStatusPlugin::onDisconnect()
{
	// This method is called when the disconnect button is pressed
	// It must remove all connections that were established on the connection
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
	return true;
}

void JesdStatusPlugin::initMetadata()
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
