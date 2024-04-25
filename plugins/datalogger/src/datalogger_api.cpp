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
	Q_ASSERT(m_dataLoggerPlugin != nullptr);
	m_dataLoggerPlugin->toggleRunState(running);
}

void DataLogger_API::clearData()
{
	Q_ASSERT(m_dataLoggerPlugin->m_dataAcquisitionManager != nullptr);
	m_dataLoggerPlugin->m_dataAcquisitionManager->clearMonitorsData();
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
	for(ToolMenuEntry *tool : m_dataLoggerPlugin->m_toolList) {
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
		monitorTool->m_dataMonitorSettings->dataLoggingMenu->dataLoggingFilePath->getLineEdit()->setText(path);
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
