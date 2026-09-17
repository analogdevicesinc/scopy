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
#include <iiowidgetbuilder.h>
#include <style.h>
#include <component/device.h>
#include <component/controller.h>
#include <component/attribute.h>

#include "core/deviceimpl.h"
#include <datalogger/datalogger_api.hpp>
#include <datalogger/dataloggerplugin.h>
#include <core/pluginrepository.h>
#include <datamonitor/sevensegmentdisplay.hpp>
#include <pluginbase/statusbarmanager.h>

Q_LOGGING_CATEGORY(CAT_RFPOWERMETER, "RFPowerMeterPlugin")
using namespace scopy::rfpowermeter;

bool RFPowerMeterPlugin::compatible(QString m_param, QString category)
{
	bool ret = false;
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		return false;
	}
	component::Device *dev = ctx->findChild<component::Device *>("powrms", Qt::FindDirectChildrenOnly);
	if(dev) {
		ret = true;
		qDebug(CAT_RFPOWERMETER) << "Found rf powermeter device";
	}

	return ret;
}

bool RFPowerMeterPlugin::loadPage() { return false; }

bool RFPowerMeterPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

QString RFPowerMeterPlugin::description() { return "RF Power Meter Plugin"; }

bool RFPowerMeterPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is
	// compatible to that device
	// In case of success the function must return true and false otherwise

	// Check for RF power meter device and apply configuration
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		return false;
	}
	component::Device *powrmsDev = ctx->findChild<component::Device *>("powrms", Qt::FindDirectChildrenOnly);
	if(!powrmsDev) {
		qDebug(CAT_RFPOWERMETER) << "RF Powermeter device not found";
		return false;
	}

	qDebug(CAT_RFPOWERMETER) << "Found rf powermeter device, configuring via DataLogger API";

	// Get DataLogger API with proper null checking
	datamonitor::DataLogger_API *dataloggerApi = nullptr;
	datamonitor::DataLoggerPlugin *dataLogger = nullptr;

	if(m_device == nullptr) {
		qDebug(CAT_RFPOWERMETER) << "Device not found";
		return false;
	}
	dataLogger = dynamic_cast<datamonitor::DataLoggerPlugin *>(m_device->getPluginByName("DataLoggerPlugin"));

	if(!dataLogger) {
		qWarning(CAT_RFPOWERMETER) << "DataLogger plugin not found or not enabled";
		StatusBarManager::pushMessage("RF Power Meter requires DataLogger plugin to be enabled. Please enable "
					      "DataLogger plugin first.",
					      8000);
		return false;
	}

	dataloggerApi = dataLogger->getApi();
	if(!dataloggerApi) {
		qWarning(CAT_RFPOWERMETER) << "DataLogger API not available";
		StatusBarManager::pushMessage("DataLogger API not available for RF Power Meter configuration.", 5000);
		return false;
	}

	if(dataLogger->toolList().isEmpty()) {
		qWarning(CAT_RFPOWERMETER) << "DataLogger has no tools available";
		StatusBarManager::pushMessage("DataLogger has no tools available for RF Power Meter.", 5000);
		return false;
	}

	// Additional safety check
	ToolMenuEntry *firstTool = dataLogger->toolList().first();
	if(!firstTool || !firstTool->tool()) {
		qWarning(CAT_RFPOWERMETER) << "DataLogger first tool is invalid";
		StatusBarManager::pushMessage("DataLogger tool is invalid for RF Power Meter.", 5000);
		return false;
	}

	datamonitor::DatamonitorTool *tool = dynamic_cast<datamonitor::DatamonitorTool *>(firstTool->tool());

	if(!tool) {
		return false;
	}

	QWidget *widget = new QWidget(tool);
	QHBoxLayout *layout = new QHBoxLayout();
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(0);
	widget->setLayout(layout);

	Style::setBackgroundColor(widget, json::theme::background_primary, true);

	IIOWidget *frequency = IIOWidgetBuilder(tool)
				       .attribute(powrmsDev->findChild<component::Attribute *>(
					       "frequency_MHz", Qt::FindDirectChildrenOnly))
				       .buildSingle();

	if(frequency != nullptr) {
		layout->addWidget(frequency);
	}

	auto *sevenSegMonitors = tool->getSevenSegmetMonitors();
	if(!sevenSegMonitors) {
		qWarning(CAT_RFPOWERMETER) << "SevenSegment monitors not available";
		StatusBarManager::pushMessage("SevenSegment monitors not available for RF Power Meter.", 5000);
		return false;
	}

	auto *sevenSegLayout = sevenSegMonitors->getLayout();
	if(!sevenSegLayout) {
		qWarning(CAT_RFPOWERMETER) << "SevenSegment layout not available";
		StatusBarManager::pushMessage("SevenSegment layout not available for RF Power Meter.", 5000);
		return false;
	}

	sevenSegLayout->addWidget(widget);

	// Configure DataLogger for RF Power Meter (matching original JavaScript)
	dataloggerApi->setToolName("Data Logger ", "RF Power Meter");
	dataloggerApi->setDisplayMode("RF Power Meter", 2);

	// Configure power5 (Power Forward)
	dataloggerApi->setMonitorDisplayName("RF Power Meter", "powrms:power5", "Power Forward");
	dataloggerApi->setMonitorUnitOfMeasurementName("RF Power Meter", "powrms:power5", "");
	dataloggerApi->setMonitorUnitOfMeasurementSymbol("RF Power Meter", "powrms:power5", "dBm");
	dataloggerApi->enableMonitorOfTool("RF Power Meter", "powrms:power5");

	// Configure power6 (Power Reverse)
	dataloggerApi->setMonitorDisplayName("RF Power Meter", "powrms:power6", "Power Reverse");
	dataloggerApi->setMonitorUnitOfMeasurementName("RF Power Meter", "powrms:power6", "");
	dataloggerApi->setMonitorUnitOfMeasurementSymbol("RF Power Meter", "powrms:power6", "dBm");
	dataloggerApi->enableMonitorOfTool("RF Power Meter", "powrms:power6");

	// Configure voltage0 (Voltage in corrected)
	dataloggerApi->setMonitorDisplayName("RF Power Meter", "powrms:voltage0", "Voltage In Corrected");
	dataloggerApi->setMonitorUnitOfMeasurementName("RF Power Meter", "powrms:voltage0", "Voltage");
	dataloggerApi->setMonitorUnitOfMeasurementSymbol("RF Power Meter", "powrms:voltage0", "mV");

	// Configure voltage1 (Voltage out corrected)
	dataloggerApi->setMonitorDisplayName("RF Power Meter", "powrms:voltage1", "Voltage Out Corrected");
	dataloggerApi->setMonitorUnitOfMeasurementName("RF Power Meter", "powrms:voltage1", "Voltage");
	dataloggerApi->setMonitorUnitOfMeasurementSymbol("RF Power Meter", "powrms:voltage1", "mV");

	// Configure voltage2 (Temperature)
	dataloggerApi->setMonitorDisplayName("RF Power Meter", "powrms:voltage2", "Temperature");
	dataloggerApi->setMonitorUnitOfMeasurementName("RF Power Meter", "powrms:voltage2", "Voltage");
	dataloggerApi->setMonitorUnitOfMeasurementSymbol("RF Power Meter", "powrms:voltage2", "mV");

	// Configure voltage3 (Voltage in)
	dataloggerApi->setMonitorDisplayName("RF Power Meter", "powrms:voltage3", "Voltage In");
	dataloggerApi->setMonitorUnitOfMeasurementName("RF Power Meter", "powrms:voltage3", "Voltage");
	dataloggerApi->setMonitorUnitOfMeasurementSymbol("RF Power Meter", "powrms:voltage3", "mV");

	// Configure voltage4 (Voltage out)
	dataloggerApi->setMonitorDisplayName("RF Power Meter", "powrms:voltage4", "Voltage Out");
	dataloggerApi->setMonitorUnitOfMeasurementName("RF Power Meter", "powrms:voltage4", "Voltage");
	dataloggerApi->setMonitorUnitOfMeasurementSymbol("RF Power Meter", "powrms:voltage4", "mV");

	qInfo(CAT_RFPOWERMETER) << "RF power meter configuration completed successfully";

	return true;
}

bool RFPowerMeterPlugin::onDisconnect() { return true; }

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
