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

#include "adrv9002plugin.h"

#include <QLoggingCategory>
#include <QLabel>
#include <iioutil/connectionprovider.h>
#include <iio.h>

#include "adrv9002.h"

Q_LOGGING_CATEGORY(CAT_ADRV9002PLUGIN, "Adrv9002Plugin")
using namespace scopy::adrv9002;

bool Adrv9002Plugin::compatible(QString m_param, QString category)
{
	// This function defines the characteristics according to which the
	// plugin is compatible with a specific device
	bool ret = false;
	auto &&cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(!conn) {
		qDebug(CAT_ADRV9002PLUGIN) << "The context is not compatible with the ADRV9002 Plugin!";
		return ret;
	}

	// Check for ADRV9002 family devices - any one of these makes the plugin compatible
	// Based on iio-oscilloscope adrv9002.c device list
	const char *adrv9002_devices[] = {"adrv9002-phy", "adrv9003-phy", "adrv9004-phy", "adrv9005-phy",
					  "adrv9006-phy"};

	iio_device *adrv9002Device = nullptr;
	for(const char *device_name : adrv9002_devices) {
		adrv9002Device = iio_context_find_device(conn->context(), device_name);
		if(adrv9002Device) {
			qDebug(CAT_ADRV9002PLUGIN) << "Found compatible device:" << device_name;
			ret = true;
			break; // Only one device needed for compatibility
		}
	}

	if(!ret) {
		qDebug(CAT_ADRV9002PLUGIN) << "No ADRV9002 family devices found in context";
	}

	cp->close(m_param);
	return ret;
}

bool Adrv9002Plugin::loadPage()
{
	// Here you must write the code for the plugin info page
	// Below is an example for an iio device
	/*m_page = new QWidget();
	m_page->setLayout(new QVBoxLayout(m_page));
	m_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_infoPage = new InfoPage(m_page);
	m_infoPage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_page->layout()->addWidget(m_infoPage);
	m_page->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

	auto cp = ContextProvider::GetInstance();
	struct iio_context *context = cp->open(m_param);
	ssize_t attributeCount = iio_context_get_attrs_count(context);
	for(int i = 0; i < attributeCount; ++i) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(context, i, &name, &value);
		if(ret < 0) {
			qWarning(CAT_ADRV9002PLUGIN) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}
	cp->close(m_param);
	m_page->ensurePolished();
	return true;
	*/
	return false;
}

bool Adrv9002Plugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void Adrv9002Plugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("adrv9002tool", "ADRV9002", ":/gui/icons/scopy-default/icons/gear_wheel.svg"));
}

void Adrv9002Plugin::unload()
{ /*delete m_infoPage;*/
}

QString Adrv9002Plugin::description() { return "ADRV9002 RF transceiver plugin ported from iio-oscilloscope"; }

bool Adrv9002Plugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is
	// compatible to that device
	// In case of success the function must return true and false otherwise

	auto &&cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);

	if(!conn) {
		qWarning(CAT_ADRV9002PLUGIN) << "No context available for ADRV9002";
		return false;
	}

	Adrv9002 *adrv9002 = new Adrv9002(conn->context());
	m_toolList[0]->setTool(adrv9002);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);
	return true;
}

bool Adrv9002Plugin::onDisconnect()
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

	auto &&cp = ConnectionProvider::GetInstance();
	cp->close(m_param);
	return true;
}

void Adrv9002Plugin::initMetadata()
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
