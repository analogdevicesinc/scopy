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

#include "dataloggerplugin.h"

#include <QLoggingCategory>
#include <QLabel>
#include <dmm.hpp>
#include <menusectionwidget.h>
#include <menucollapsesection.h>
#include <timemanager.hpp>
#include <datamonitorutils.hpp>
#include <datalogger_api.hpp>

#include <libm2k/analog/dmm.hpp>

#include <iioutil/connectionprovider.h>

#include <pluginbase/preferences.h>
#include <gui/preferenceshelper.h>

#include <pluginbase/scopyjs.h>

Q_LOGGING_CATEGORY(CAT_DATALOGGERLUGIN, "DataLoggerPlugin")
using namespace scopy::datamonitor;

bool DataLoggerPlugin::compatible(QString m_param, QString category)
{
	// This function defines the characteristics according to which the
	// plugin is compatible with a specific device
	m_name = "DataLogger";
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);

	if(!conn) {
		qWarning(CAT_DATALOGGERLUGIN) << "No context available for datalogger";
		return false;
	}

	cp->close(m_param);

	return true;
}

bool DataLoggerPlugin::loadPage() { return false; }

bool DataLoggerPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/scopy-default/icons/datalogger.svg");
	return true;
}

void DataLoggerPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("DataMonitorPreview", "Data Logger", toolIcon));
}

void DataLoggerPlugin::unload()
{ /*delete m_infoPage;*/
}

bool DataLoggerPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is
	// compatible to that device
	// In case of success the function must return true and false otherwise

	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(conn == nullptr)
		return false;
	iio_context *ctx = conn->context();

	DMM dmm;
	QList<DmmDataMonitorModel *> availableDmmList = dmm.getDmmMonitors(ctx);

	foreach(auto dmmModel, availableDmmList) {
		dmmList.push_back(dmmModel);
	}

	m_dataAcquisitionManager = new DataAcquisitionManager(this);

	foreach(ReadableDataMonitorModel *monitor, dmmList) {
		m_dataAcquisitionManager->getDataMonitorMap()->insert(monitor->getName(), monitor);
	}

	Preferences *p = Preferences::GetInstance();

	auto &&timeTracker = TimeManager::GetInstance();
	timeTracker->setTimerInterval(p->get("dataloggerplugin_read_interval").toDouble());

	connect(timeTracker, &TimeManager::timeout, m_dataAcquisitionManager, &DataAcquisitionManager::readData);

	QObject::connect(p, &Preferences::preferenceChanged, this, [=, this](QString id, QVariant var) {
		if(id.contains("dataloggerplugin_read_interval")) {
			timeTracker->setTimerInterval(p->get("dataloggerplugin_read_interval").toDouble());
		}
	});

	removeTool("DataMonitorPreview");
	addNewTool();
	initApi();

	return true;
}

bool DataLoggerPlugin::onDisconnect()
{
	auto count = dmmList.count();
	for(int i = 0; i < count; i++) {
		delete dmmList.takeLast();
	}

	if(m_dataAcquisitionManager) {
		delete m_dataAcquisitionManager;
		m_dataAcquisitionManager = nullptr;
	}

	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	cp->close(m_param);

	// This method is called when the disconnect button is pressed
	// It must remove all connections that were established on the connection
	while(!m_toolList.isEmpty()) {
		ToolMenuEntry *tool = m_toolList.first();
		tool->setEnabled(false);
		tool->setRunning(false);
		tool->setRunBtnVisible(false);
		removeTool(tool->id());
	}

	// add proxy tool to represent the plugin
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("DataMonitorPreview", "Data Logger", toolIcon));

	Q_EMIT toolListChanged();

	toolIndex = 0;
	return true;
}

void DataLoggerPlugin::addNewTool()
{
	QString tool_name = QString("Data Logger ");
	if(toolIndex != 0) {
		tool_name += QString::number(toolIndex);
	}

	ToolMenuEntry *toolMenuEntry = SCOPY_NEW_TOOLMENUENTRY(tool_name, tool_name, toolIcon);
	m_toolList.append(toolMenuEntry);
	m_toolList.last()->setEnabled(true);
	m_toolList.last()->setRunBtnVisible(true);

	bool isDeletable = m_toolList.length() > 1;
	DatamonitorTool *datamonitorTool = new DatamonitorTool(m_dataAcquisitionManager, isDeletable);

	connect(datamonitorTool, &DatamonitorTool::requestNewTool, this, &DataLoggerPlugin::addNewTool);
	connect(datamonitorTool, &DatamonitorTool::runToggled, this, &DataLoggerPlugin::toggleRunState);

	connect(datamonitorTool, &DatamonitorTool::requestDeleteTool, this, [=, this]() {
		// make sure at least one tool exists
		if(m_toolList.length() > 1) {
			removeTool(tool_name);
			requestTool(m_toolList.first()->id());
		}
	});
	connect(datamonitorTool, &DatamonitorTool::settingsTitleChanged, this,
		[=, this](QString newTitle) { toolMenuEntry->setName(newTitle); });

	datamonitorTool->getRunButton()->setChecked(isRunning);

	// one for each
	connect(toolMenuEntry, &ToolMenuEntry::runToggled, this, [=, this](bool en) {
		if(datamonitorTool->getRunButton()->isChecked() != en) {
			datamonitorTool->getRunButton()->toggle();
		}
	});

	toolMenuEntry->setDetachable(false);

	Q_EMIT toolListChanged();
	m_toolList.last()->setTool(datamonitorTool);
	if(m_toolList.length() > 1) {
		requestTool(tool_name);
	}

	toolIndex++;
}

void DataLoggerPlugin::removeTool(QString toolId)
{

	auto *tool = ToolMenuEntry::findToolMenuEntryById(m_toolList, toolId);
	m_toolList.removeOne(tool);
	QWidget *datamonitorTool = tool->tool();
	tool->setTool(nullptr);
	if(datamonitorTool) {
		delete datamonitorTool;
	}

	Q_EMIT toolListChanged();
	tool->deleteLater();
}

void DataLoggerPlugin::toggleRunState(bool toggled)
{
	for(int i = 0; i < m_toolList.length(); i++) {
		m_toolList[i]->setRunning(toggled);
	}
	isRunning = toggled;
}

void DataLoggerPlugin::initApi()
{
	api = new DataLogger_API(this);
	ScopyJS *js = ScopyJS::GetInstance();
	api->setObjectName("datalogger");
	js->registerApi(api);
}

void DataLoggerPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":3,
	   "category":[
		  "iio"
	   ],
	   "exclude":[""]
	}
)plugin");
}

void DataLoggerPlugin::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->init("dataloggerplugin_data_storage_size", "10 Kb");
	p->init("dataloggerplugin_read_interval", "1");
	p->init("dataloggerplugin_date_time_format", "hh:mm:ss");
	p->init("dataloggerplugin_start_tutorial", true);
}

bool DataLoggerPlugin::loadPreferencesPage()
{
	Preferences *p = Preferences::GetInstance();

	m_preferencesPage = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_preferencesPage);

	MenuSectionWidget *generalWidget = new MenuSectionWidget(m_preferencesPage);
	MenuCollapseSection *generalSection = new MenuCollapseSection(
		"General", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, generalWidget);
	generalWidget->contentLayout()->setSpacing(10);
	generalWidget->contentLayout()->addWidget(generalSection);
	generalSection->contentLayout()->setSpacing(10);
	lay->setMargin(0);
	lay->addWidget(generalWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCombo(
		p, "dataloggerplugin_data_storage_size", "Maximum data stored for each monitor", {"10 Kb", "1 Mb"},
		generalSection));

	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceEdit(
		p, "dataloggerplugin_read_interval", "Read interval (seconds) ", generalSection));

	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceEdit(
		p, "dataloggerplugin_date_time_format", "DateTime format :", generalSection));

	QWidget *resetTutorialWidget = new QWidget();
	QHBoxLayout *resetTutorialWidgetLayout = new QHBoxLayout();

	resetTutorialWidget->setLayout(resetTutorialWidgetLayout);
	resetTutorialWidgetLayout->setMargin(0);

	QPushButton *resetTutorial = new QPushButton("Reset", generalSection);
	StyleHelper::BlueButton(resetTutorial, "resetBtn");
	connect(resetTutorial, &QPushButton::clicked, this,
		[=, this]() { p->set("dataloggerplugin_start_tutorial", true); });

	resetTutorialWidgetLayout->addWidget(new QLabel("Data logger tutorial "), 6);
	resetTutorialWidgetLayout->addWidget(resetTutorial, 1);
	generalSection->contentLayout()->addWidget(resetTutorialWidget);

	return true;
}

QString DataLoggerPlugin::description() { return "Use IIO raw and scale attributes to plot and save data"; }

#include "moc_dataloggerplugin.cpp"
