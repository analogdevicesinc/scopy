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

#include "datalogger_api.hpp"
#include "menus/datamonitorsettings.hpp"

using namespace scopy::datamonitor;

Q_LOGGING_CATEGORY(CAT_DATAMONITOR_API, "DataMonitor_API")

DataLogger_API::DataLogger_API(DataLoggerPlugin *dataLoggerPlugin)
	: ApiObject()
	, m_dataLoggerPlugin(dataLoggerPlugin)
{}

DataLogger_API::~DataLogger_API() {}

QString DataLogger_API::showAvailableDevices()
{
	Q_ASSERT(m_dataLoggerPlugin->m_dataAcquisitionManager != nullptr);
	QString availableDevices = "";
	foreach(QString monitor, m_dataLoggerPlugin->m_dataAcquisitionManager->getDataMonitorMap()->keys()) {
		if(!availableDevices.contains(monitor)) {
			availableDevices += monitor + "\n";
		}
	}
	return availableDevices;
}

QString DataLogger_API::showMonitorsOfDevice(QString device)
{
	Q_ASSERT(m_dataLoggerPlugin->m_dataAcquisitionManager != nullptr);
	QString availableDevices = "";

	foreach(QString monitor, m_dataLoggerPlugin->m_dataAcquisitionManager->getDataMonitorMap()->keys()) {
		if(monitor.contains(device)) {
			availableDevices += m_dataLoggerPlugin->m_dataAcquisitionManager->getDataMonitorMap()
						    ->value(monitor)
						    ->getName() +
				"\n";
		}
	}

	return availableDevices;
}

QString DataLogger_API::showAvailableMonitors()
{
	Q_ASSERT(m_dataLoggerPlugin->m_dataAcquisitionManager != nullptr);
	QString availableDevices = "";

	foreach(QString monitor, m_dataLoggerPlugin->m_dataAcquisitionManager->getDataMonitorMap()->keys()) {
		availableDevices += monitor + "\n";
	}

	return availableDevices;
}

QString DataLogger_API::enableMonitor(QString monitor)
{
	Q_ASSERT(m_dataLoggerPlugin->m_dataAcquisitionManager != nullptr);

	if(!m_dataLoggerPlugin->m_dataAcquisitionManager->getDataMonitorMap()->contains(monitor)) {
		return "Selected monitor dosen't exists";
	} else {
		m_dataLoggerPlugin->m_dataAcquisitionManager->updateActiveMonitors(true, monitor);
	}

	return "Success";
}

QString DataLogger_API::disableMonitor(QString monitor)
{
	Q_ASSERT(m_dataLoggerPlugin->m_dataAcquisitionManager != nullptr);

	if(!m_dataLoggerPlugin->m_dataAcquisitionManager->getActiveMonitors().contains(monitor)) {
		return "Selected monitor dosen't exists or is already disabled";
	} else {
		m_dataLoggerPlugin->m_dataAcquisitionManager->updateActiveMonitors(false, monitor);
	}

	return "Success";
}

void DataLogger_API::setRunning(bool running)
{
	Q_ASSERT(m_dataLoggerPlugin->m_dataAcquisitionManager != nullptr);
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, "Data Logger ");
	if(tool) {
		tool->setRunning(running);
		tool->runToggled(running);
	}
}

void DataLogger_API::clearData()
{
	Q_ASSERT(m_dataLoggerPlugin->m_dataAcquisitionManager != nullptr);
	m_dataLoggerPlugin->m_dataAcquisitionManager->clearMonitorsData();
}

void DataLogger_API::changeTool(QString name)
{
	Q_ASSERT(!m_dataLoggerPlugin->m_toolList.isEmpty());

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, "Data Logger ");
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(name == "Plot") {
			monitorTool->showPlot->click();
		} else if(name == "Text") {
			monitorTool->showText->click();
		} else if(name == "7 Segment") {
			monitorTool->showSegments->click();
		} else {
			qWarning(CAT_DATAMONITOR_API) << "No tool available with name " << name;
		}
	}
}

void DataLogger_API::setMinMax(bool enable)
{
	Q_ASSERT(!m_dataLoggerPlugin->m_toolList.isEmpty());

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, "Data Logger ");
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		SevenSegmentMonitorSettings *monitorSettings =
			monitorTool->m_dataMonitorSettings->getSevenSegmentMonitorSettings();
		monitorTool->sevenSegmetMonitors->togglePeakHolder(enable);
		monitorSettings->peakHolderToggle->onOffswitch()->setChecked(enable);
	}
}

void DataLogger_API::changePrecision(int decimals)
{
	Q_ASSERT(!m_dataLoggerPlugin->m_toolList.isEmpty());

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, "Data Logger ");
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		SevenSegmentMonitorSettings *monitorSettings =
			monitorTool->m_dataMonitorSettings->getSevenSegmentMonitorSettings();
		monitorTool->sevenSegmetMonitors->updatePrecision(decimals);
		monitorSettings->precision->setText(QString::number(decimals));
	}
}

void DataLogger_API::setMinYAxis(double min)
{
	Q_ASSERT(!m_dataLoggerPlugin->m_toolList.isEmpty());

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, "Data Logger ");
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(monitorTool && monitorTool->m_plotManager) {
			for(PlotComponent *pc : monitorTool->m_plotManager->plots()) {
				MonitorPlot *plot = dynamic_cast<MonitorPlot *>(pc);
				if(plot) {
					plot->updateYAxisIntervalMin(min);
				}
			}
		}
	}
}

void DataLogger_API::setMaxYAxis(double max)
{
	Q_ASSERT(!m_dataLoggerPlugin->m_toolList.isEmpty());

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, "Data Logger ");
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(monitorTool && monitorTool->m_plotManager) {
			for(PlotComponent *pc : monitorTool->m_plotManager->plots()) {
				MonitorPlot *plot = dynamic_cast<MonitorPlot *>(pc);
				if(plot) {
					plot->updateYAxisIntervalMax(max);
				}
			}
		}
	}
}

QString DataLogger_API::createTool()
{
	Q_ASSERT(m_dataLoggerPlugin != nullptr);
	m_dataLoggerPlugin->addNewTool();

	return "Tool created";
}

QString DataLogger_API::getToolList()
{
	Q_ASSERT(!m_dataLoggerPlugin->m_toolList.isEmpty());
	QString tools = "";
	for(ToolMenuEntry *tool : qAsConst(m_dataLoggerPlugin->m_toolList)) {
		if(tool->pluginName() == "DataLoggerPlugin") {
			tools += tool->name() + "\n";
		}
	}
	return tools;
}

QString DataLogger_API::enableMonitorOfTool(QString toolName, QString monitor)
{
	Q_ASSERT(!m_dataLoggerPlugin->m_toolList.isEmpty());

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(!monitorTool->m_dataAcquisitionManager->getDataMonitorMap()->contains(monitor)) {
			return "Selected monitor dosen't exists";
		} else {
			Q_EMIT monitorTool->m_monitorSelectionMenu->requestMonitorToggled(true, monitor);
		}
	}
	return "OK";
}

QString DataLogger_API::disableMonitorOfTool(QString toolName, QString monitor)
{
	Q_ASSERT(!m_dataLoggerPlugin->m_toolList.isEmpty());

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(!monitorTool->m_dataAcquisitionManager->getDataMonitorMap()->contains(monitor)) {
			return "Selected monitor dosen't exists";
		} else {
			Q_EMIT monitorTool->m_monitorSelectionMenu->requestMonitorToggled(false, monitor);
		}
	}
	return "OK";
}

void DataLogger_API::setLogPathOfTool(QString toolName, QString path)
{
	Q_ASSERT(!m_dataLoggerPlugin->m_toolList.isEmpty());

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		monitorTool->m_dataMonitorSettings->dataLoggingMenu->filename = path;
		monitorTool->m_dataMonitorSettings->dataLoggingMenu->fileBrowser->lineEdit()->setText(path);
	}
}

void DataLogger_API::logAtPathForTool(QString toolName, QString path)
{
	Q_ASSERT(!m_dataLoggerPlugin->m_toolList.isEmpty());

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		Q_EMIT monitorTool->m_dataMonitorSettings->dataLoggingMenu->requestDataLogging(path);
	}
}

void DataLogger_API::continuousLogAtPathForTool(QString toolName, QString path)
{
	Q_ASSERT(!m_dataLoggerPlugin->m_toolList.isEmpty());

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		setLogPathOfTool(toolName, path);
		monitorTool->m_dataMonitorSettings->dataLoggingMenu->liveDataLoggingButton->onOffswitch()->setChecked(
			true);
	}
}

void DataLogger_API::stopContinuousLogForTool(QString toolName)
{
	Q_ASSERT(!m_dataLoggerPlugin->m_toolList.isEmpty());

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		monitorTool->m_dataMonitorSettings->dataLoggingMenu->liveDataLoggingButton->onOffswitch()->setChecked(
			false);
	}
}

void DataLogger_API::importDataFromPathForTool(QString toolName, QString path)
{
	Q_ASSERT(!m_dataLoggerPlugin->m_toolList.isEmpty());

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		setLogPathOfTool(toolName, path);
		Q_EMIT monitorTool->m_dataMonitorSettings->dataLoggingMenu->requestDataLoading(path);
	}
}

#include "moc_datalogger_api.cpp"
