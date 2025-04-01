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

#include "ad9084plugin.h"

#include <QLoggingCategory>
#include <QLabel>

#include <style.h>
#include <iioutil/connectionprovider.h>

#include "ad9084.h"

Q_LOGGING_CATEGORY(CAT_AD9084PLUGIN, "AD9084Plugin")
using namespace scopy::ad9084;

bool AD9084Plugin::compatible(QString m_param, QString category)
{
	// This function defines the characteristics according to which the
	// plugin is compatible with a specific device
	bool ret = false;
	auto &&cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(!conn) {
		qWarning() << "The context is not compatible with the AD9084 Plugin!";
		return ret;
	}

	iio_device *apolloDevice = iio_context_find_device(conn->context(), "axi-ad9084-rx-hpc");
	if(apolloDevice) {
		ret = true;
	}
	cp->close(m_param);
	return ret;
}

bool AD9084Plugin::loadPage() { return false; }

bool AD9084Plugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void AD9084Plugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("ad9084", "AD9084", ":/gui/icons/scopy-default/icons/gear_wheel.svg"));
}

void AD9084Plugin::unload()
{ /*delete m_infoPage;*/
}

QString AD9084Plugin::description() { return "Tool for Apollo MxFE Quad AD9084 interaction."; }

bool AD9084Plugin::onConnect()
{
	int deviceIdx = 1;
	auto &&cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(!conn) {
		return false;
	}

	struct iio_device *dev = iio_context_find_device(conn->context(), "axi-ad9084-rx-hpc");
	m_toolList.last()->setTool(new Ad9084(dev));
	m_toolList.last()->setEnabled(true);
	m_toolList.last()->setRunBtnVisible(true);

	unsigned int devCount = iio_context_get_devices_count(conn->context());
	for(unsigned int i = 0; i < devCount; i++) {
		struct iio_device *rxDev = iio_context_get_device(conn->context(), i);
		if(!rxDev) {
			continue;
		}
		if(rxDev == dev) {
			continue;
		}

		QString name = iio_device_get_name(rxDev);
		if(name.contains("axi-ad9084-rx")) {
			m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("ad9084", QString("AD9084 %1").arg(deviceIdx),
								  Style::getAttribute(json::theme::icon_theme_folder) +
									  "/icons/tool_network_analyzer.svg"));
			Q_EMIT toolListChanged();
			m_toolList.last()->setTool(new Ad9084(rxDev));
			m_toolList.last()->setEnabled(true);
			m_toolList.last()->setRunBtnVisible(true);
			deviceIdx++;
		}
	}
	return true;
}

bool AD9084Plugin::onDisconnect()
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

void AD9084Plugin::initMetadata()
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
