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
#include <dataloggerplugin.h>
#include <sevensegmentdisplay.hpp>
#include <sevensegmentmonitorsettings.hpp>

using namespace scopy::datamonitor;

Q_LOGGING_CATEGORY(CAT_DATAMONITOR_API, "DataMonitor_API")

DataLogger_API::DataLogger_API(DataLoggerPlugin *dataLoggerPlugin)
	: ApiObject()
	, m_dataLoggerPlugin(dataLoggerPlugin)
{}

DataLogger_API::~DataLogger_API() {}

QString DataLogger_API::showAvailableDevices()
{
	if(!m_dataLoggerPlugin || !m_dataLoggerPlugin->m_dataAcquisitionManager) {
		qWarning(CAT_DATAMONITOR_API) << "DataAcquisitionManager is null";
		return "";
	}
	QString availableDevices = "";
	foreach(QString monitor, m_dataLoggerPlugin->m_dataAcquisitionManager->getDataMonitorMap()->keys()) {
		if(!availableDevices.contains(monitor)) {
			availableDevices += monitor + "\n";
		}
	}
	return availableDevices;
}

QString DataLogger_API::showMonitorsOfDevice(const QString &device)
{
	if(!m_dataLoggerPlugin || !m_dataLoggerPlugin->m_dataAcquisitionManager) {
		qWarning(CAT_DATAMONITOR_API) << "DataAcquisitionManager is null";
		return "";
	}
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
	if(!m_dataLoggerPlugin || !m_dataLoggerPlugin->m_dataAcquisitionManager) {
		qWarning(CAT_DATAMONITOR_API) << "DataAcquisitionManager is null";
		return "";
	}
	QString availableDevices = "";

	foreach(QString monitor, m_dataLoggerPlugin->m_dataAcquisitionManager->getDataMonitorMap()->keys()) {
		availableDevices += monitor + "\n";
	}

	return availableDevices;
}

QString DataLogger_API::enableMonitor(const QString &monitor)
{
	if(!m_dataLoggerPlugin || !m_dataLoggerPlugin->m_dataAcquisitionManager) {
		qWarning(CAT_DATAMONITOR_API) << "DataAcquisitionManager is null";
		return "Error: DataAcquisitionManager is null";
	}

	if(!m_dataLoggerPlugin->m_dataAcquisitionManager->getDataMonitorMap()->contains(monitor)) {
		return "Selected monitor dosen't exists";
	} else {
		m_dataLoggerPlugin->m_dataAcquisitionManager->updateActiveMonitors(true, monitor);
	}

	return "Success";
}

QString DataLogger_API::disableMonitor(const QString &monitor)
{
	if(!m_dataLoggerPlugin || !m_dataLoggerPlugin->m_dataAcquisitionManager) {
		qWarning(CAT_DATAMONITOR_API) << "DataAcquisitionManager is null";
		return "Error: DataAcquisitionManager is null";
	}

	if(!m_dataLoggerPlugin->m_dataAcquisitionManager->getActiveMonitors().contains(monitor)) {
		return "Selected monitor dosen't exists or is already disabled";
	} else {
		m_dataLoggerPlugin->m_dataAcquisitionManager->updateActiveMonitors(false, monitor);
	}

	return "Success";
}

void DataLogger_API::setRunning(bool running)
{
	if(!m_dataLoggerPlugin || !m_dataLoggerPlugin->m_dataAcquisitionManager) {
		qWarning(CAT_DATAMONITOR_API) << "DataAcquisitionManager is null";
		return;
	}
	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, "Data Logger ");
	if(tool) {
		tool->setRunning(running);
		tool->runToggled(running);
	}
}

void DataLogger_API::clearData()
{
	if(!m_dataLoggerPlugin || !m_dataLoggerPlugin->m_dataAcquisitionManager) {
		qWarning(CAT_DATAMONITOR_API) << "DataAcquisitionManager is null";
		return;
	}
	m_dataLoggerPlugin->m_dataAcquisitionManager->clearMonitorsData();
}

void DataLogger_API::changeTool(const QString &name)
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, "Data Logger ");
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(!monitorTool) {
			qWarning(CAT_DATAMONITOR_API) << "MonitorTool is null";
			return;
		}
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
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, "Data Logger ");
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(!monitorTool) {
			qWarning(CAT_DATAMONITOR_API) << "MonitorTool is null";
			return;
		}
		SevenSegmentMonitorSettings *monitorSettings =
			monitorTool->m_dataMonitorSettings->getSevenSegmentMonitorSettings();
		monitorTool->sevenSegmetMonitors->togglePeakHolder(enable);
		monitorSettings->peakHolderToggle->onOffswitch()->setChecked(enable);
	}
}

void DataLogger_API::changePrecision(int decimals)
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, "Data Logger ");
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(!monitorTool) {
			qWarning(CAT_DATAMONITOR_API) << "MonitorTool is null";
			return;
		}
		SevenSegmentMonitorSettings *monitorSettings =
			monitorTool->m_dataMonitorSettings->getSevenSegmentMonitorSettings();
		monitorTool->sevenSegmetMonitors->updatePrecision(decimals);
		monitorSettings->precision->setText(QString::number(decimals));
	}
}

void DataLogger_API::setMinYAxis(double min)
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

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
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

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
	if(!m_dataLoggerPlugin) {
		qWarning(CAT_DATAMONITOR_API) << "DataLoggerPlugin is null";
		return "Error: Plugin is null";
	}
	m_dataLoggerPlugin->addNewTool();

	return "Tool created";
}

QString DataLogger_API::getToolList()
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return "";
	}
	QString tools = "";
	for(ToolMenuEntry *tool : qAsConst(m_dataLoggerPlugin->m_toolList)) {
		if(tool->pluginName() == "DataLoggerPlugin") {
			tools += tool->name() + "\n";
		}
	}
	return tools;
}

QString DataLogger_API::enableMonitorOfTool(const QString &toolName, const QString &monitor)
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return "Error: Plugin or tool list is null/empty";
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(!monitorTool) {
			qWarning(CAT_DATAMONITOR_API) << "MonitorTool is null";
			return "Error: MonitorTool is null";
		}
		if(!monitorTool->m_dataAcquisitionManager->getDataMonitorMap()->contains(monitor)) {
			return "Selected monitor dosen't exists";
		} else {
			Q_EMIT monitorTool->m_monitorSelectionMenu->requestMonitorToggled(true, monitor);
		}
	}
	return "OK";
}

QString DataLogger_API::disableMonitorOfTool(const QString &toolName, const QString &monitor)
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return "Error: Plugin or tool list is null/empty";
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(!monitorTool) {
			qWarning(CAT_DATAMONITOR_API) << "MonitorTool is null";
			return "Error: MonitorTool is null";
		}
		if(!monitorTool->m_dataAcquisitionManager->getDataMonitorMap()->contains(monitor)) {
			return "Selected monitor dosen't exists";
		} else {
			Q_EMIT monitorTool->m_monitorSelectionMenu->requestMonitorToggled(false, monitor);
		}
	}
	return "OK";
}

void DataLogger_API::setLogPathOfTool(const QString &toolName, const QString &path)
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(!monitorTool) {
			qWarning(CAT_DATAMONITOR_API) << "MonitorTool is null";
			return;
		}
		monitorTool->m_dataMonitorSettings->dataLoggingMenu->filename = path;
		monitorTool->m_dataMonitorSettings->dataLoggingMenu->fileBrowser->lineEdit()->setText(path);
	}
}

void DataLogger_API::logAtPathForTool(const QString &toolName, const QString &path)
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(!monitorTool) {
			qWarning(CAT_DATAMONITOR_API) << "MonitorTool is null";
			return;
		}
		Q_EMIT monitorTool->m_dataMonitorSettings->dataLoggingMenu->requestDataLogging(path);
	}
}

void DataLogger_API::continuousLogAtPathForTool(const QString &toolName, const QString &path)
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(!monitorTool) {
			qWarning(CAT_DATAMONITOR_API) << "MonitorTool is null";
			return;
		}
		setLogPathOfTool(toolName, path);
		monitorTool->m_dataMonitorSettings->dataLoggingMenu->liveDataLoggingButton->onOffswitch()->setChecked(
			true);
	}
}

void DataLogger_API::stopContinuousLogForTool(const QString &toolName)
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(!monitorTool) {
			qWarning(CAT_DATAMONITOR_API) << "MonitorTool is null";
			return;
		}
		monitorTool->m_dataMonitorSettings->dataLoggingMenu->liveDataLoggingButton->onOffswitch()->setChecked(
			false);
	}
}

void DataLogger_API::importDataFromPathForTool(const QString &toolName, const QString &path)
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool) {
		DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());
		if(!monitorTool) {
			qWarning(CAT_DATAMONITOR_API) << "MonitorTool is null";
			return;
		}
		setLogPathOfTool(toolName, path);
		Q_EMIT monitorTool->m_dataMonitorSettings->dataLoggingMenu->requestDataLoading(path);
	}
}

void DataLogger_API::setMonitorDisplayName(const QString &toolName, const QString &monitor, const QString &displayName)
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool == nullptr) {
		return;
	}
	DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());

	if(monitorTool == nullptr)
		return;

	if(!monitorTool->m_dataAcquisitionManager->getDataMonitorMap()->contains(monitor))
		return;

	monitorTool->m_dataAcquisitionManager->getDataMonitorMap()->value(monitor)->setDisplayName(displayName);
}

void DataLogger_API::setMonitorUnitOfMeasurementName(const QString &toolName, const QString &monitor,
						     const QString &name)
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool == nullptr) {
		return;
	}
	DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());

	if(monitorTool == nullptr)
		return;

	if(!monitorTool->m_dataAcquisitionManager->getDataMonitorMap()->contains(monitor))
		return;

	monitorTool->m_dataAcquisitionManager->getDataMonitorMap()->value(monitor)->getUnitOfMeasure()->setName(name);
}

void DataLogger_API::setMonitorUnitOfMeasurementSymbol(const QString &toolName, const QString &monitor,
						       const QString &symbol)
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(tool == nullptr)
		return;

	DatamonitorTool *monitorTool = dynamic_cast<DatamonitorTool *>(tool->tool());

	if(monitorTool == nullptr)
		return;

	if(!monitorTool->m_dataAcquisitionManager->getDataMonitorMap()->contains(monitor))
		return;

	monitorTool->m_dataAcquisitionManager->getDataMonitorMap()->value(monitor)->getUnitOfMeasure()->setSymbol(
		symbol);
}

void DataLogger_API::setDisplayMode(const QString &toolName, int mode)
{
	// mode: 0 = plot, 1 = text, 2 = seven segment
	if(mode < 0 || mode > 2) {
		qWarning(CAT_DATAMONITOR_API) << "Invalid display mode:" << mode;
		return;
	}

	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, toolName);
	if(!tool) {
		qWarning(CAT_DATAMONITOR_API) << "Tool not found:" << toolName;
		return;
	}

	DatamonitorTool *monitorTool = qobject_cast<DatamonitorTool *>(tool->tool());
	if(monitorTool) {
		monitorTool->setDisplayMode(mode);
	}
}

void DataLogger_API::setToolName(const QString &currentToolName, const QString &newToolName)
{
	if(!m_dataLoggerPlugin || m_dataLoggerPlugin->m_toolList.isEmpty()) {
		qWarning(CAT_DATAMONITOR_API) << "Plugin or tool list is null/empty";
		return;
	}

	ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(m_dataLoggerPlugin->m_toolList, currentToolName);
	if(!tool) {
		qWarning(CAT_DATAMONITOR_API) << "Tool not found:" << currentToolName;
		return;
	}

	tool->setName(newToolName);
}

#include "moc_datalogger_api.cpp"
