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

#include "rfpowermeter.h"

#include <QLoggingCategory>
#include <QLabel>
#include <iioutil/connectionprovider.h>
#include <pluginbase/scopyjs.h>

Q_LOGGING_CATEGORY(CAT_RFPOWERMETER, "RFPowerMeterPlugin")
using namespace scopy::rfpowermeter;

bool RFPowerMeterPlugin::compatible(QString m_param, QString category)
{
	// This function defines the characteristics according to which the
	// plugin is compatible with a specific device
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(conn == nullptr)
		return false;
	iio_context *ctx = conn->context();
	iio_device *powrmsDevice = iio_context_find_device(ctx, "powrms");
	if(powrmsDevice != nullptr) {
		qDebug(CAT_RFPOWERMETER) << "Found rf powermeter device";
	}
	bool ret = true;
	return ret;
}

bool RFPowerMeterPlugin::loadPage()
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
			qWarning(CAT_RFPOWERMETER) << "Could not read attribute with index:" << i;
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

bool RFPowerMeterPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void RFPowerMeterPlugin::loadToolList() {}

void RFPowerMeterPlugin::unload()
{ /*delete m_infoPage;*/
}

QString RFPowerMeterPlugin::description() { return "RF Power Meter Plugin"; }

bool RFPowerMeterPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is
	// compatible to that device
	// In case of success the function must return true and false otherwise

	// Check for RF power meter device and apply configuration
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(conn == nullptr)
		return false;
	iio_context *ctx = conn->context();

	iio_device *powrmsDevice = iio_context_find_device(ctx, "powrms");
	if(powrmsDevice != nullptr) {
		qDebug(CAT_RFPOWERMETER) << "Found rf powermeter device, applying configuration via JavaScript";

		// Execute the JavaScript configuration script
		ScopyJS *js = ScopyJS::GetInstance();
		if(!js) {
			qWarning(CAT_RFPOWERMETER) << "ScopyJS instance not available";
			return false;
		}

		QJSValue result = js->evaluateFile(":/rfpowermeter/rfpowermeter_config.js");
		if(result.isError()) {
			qWarning(CAT_RFPOWERMETER)
				<< "Error executing RF power meter configuration script:" << result.toString();
			return false;
		}

		qInfo(CAT_RFPOWERMETER) << "RF power meter configuration script executed successfully";
	}

	return true;
}

bool RFPowerMeterPlugin::onDisconnect()
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

void RFPowerMeterPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":2,
	   "category":[
	      "iio"
	   ],
	   "exclude":[""]
	}
)plugin");
}
