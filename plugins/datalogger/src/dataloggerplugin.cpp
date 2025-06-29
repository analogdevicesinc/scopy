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
#include <style.h>
#include <deviceiconbuilder.h>

#include <libm2k/analog/dmm.hpp>

#include <iioutil/connectionprovider.h>

#include <pluginbase/preferences.h>
#include <gui/preferenceshelper.h>

#include <pluginbase/scopyjs.h>
#include <scopy-datalogger_config.h>

Q_LOGGING_CATEGORY(CAT_DATALOGGERLUGIN, "DataLoggerPlugin")
using namespace scopy::datamonitor;

bool DataLoggerPlugin::compatible(QString m_param, QString category)
{
	// This function defines the characteristics according to which the
	// plugin is compatible with a specific device
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
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("DLOG");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();
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
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("DataMonitorPreview", "Data Logger",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_datalogger.svg"));

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

	Q_EMIT toolListChanged();
	m_toolList.last()->setTool(datamonitorTool);
	if(m_toolList.length() > 1) {
		toolMenuEntry->setDetachable(false);
		requestTool(tool_name);
	}

	toolIndex++;

	datamonitorTool->setEnableAddRemovePlot(Preferences::get("dataloggerplugin_add_remove_plot").toBool());
	datamonitorTool->setEnableAddRemoveInstrument(
		Preferences::get("dataloggerplugin_add_remove_instrument").toBool());
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

		// Tool run/stop btn state sync
		auto *toolWidget = qobject_cast<DatamonitorTool *>(m_toolList[i]->tool());
		if(toolWidget && toolWidget->getRunButton()->isChecked() != toggled) {
			toolWidget->getRunButton()->setChecked(toggled);
		}
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
	   ]
	}
)plugin");
}

void DataLoggerPlugin::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->init("dataloggerplugin_data_storage_size", "10 Kb");
	p->init("dataloggerplugin_read_interval", "1");
	p->init("dataloggerplugin_date_time_format", "hh:mm:ss");
	p->init("dataloggerplugin_add_remove_plot", false);
	p->init("dataloggerplugin_add_remove_instrument", false);
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

	QStringList storage_options = {"10 Kb", "1 Mb"};
	generalSection->contentLayout()->addWidget(
		PREFERENCE_COMBO(p, "dataloggerplugin_data_storage_size", "Maximum data stored for each monitor",
				 "Select the maximum data storage size for each monitor in the datalogger.",
				 storage_options, generalSection));

	generalSection->contentLayout()->addWidget(PREFERENCE_EDIT_VALIDATION(
		p, "dataloggerplugin_read_interval", "Read interval (seconds) ",
		"Select the time interval, in seconds, for data polling in the instrument.",
		[](const QString &text) {
			// check if input is an positive integer
			bool ok;
			auto value = text.toInt(&ok);
			return ok && value >= 0;
		},
		generalSection));

	generalSection->contentLayout()->addWidget(PREFERENCE_EDIT_VALIDATION(
		p, "dataloggerplugin_date_time_format",
		"DateTime format :", "Select the date time format of the instrument. Default value is: hh:mm:ss",
		[](const QString &text) {
			// check if input is a valid datetime format
			QRegularExpression pattern(
				R"(^((YYYY-MM-DD)|(YYYY-MM-DD hh:mm:ss)|(YYYY-MM-DD mm:ss)|(YYYY-MM-DD ss)|(hh:mm:ss)|(mm:ss)|(ss))$)");
			return pattern.match(text).hasMatch();
		},
		generalSection));

	generalSection->contentLayout()->addWidget(
		PREFERENCE_CHECK_BOX(p, "dataloggerplugin_add_remove_plot", "Add/Remove plot feature (EXPERIMENTAL)",
				     "Experimental feature allowing the user to create multiple time or frequency plots"
				     "within the ADC instrument. Any channel can be moved between plots to provide "
				     "visual clarity and separation.",
				     m_preferencesPage));
	generalSection->contentLayout()->addWidget(PREFERENCE_CHECK_BOX(
		p, "dataloggerplugin_add_remove_instrument", "Add/Remove instrument feature (EXPERIMENTAL)",
		"Experimental feature allowing the user to create multiple time or frequency "
		"instruments, providing an easy way to switch between different run scenarios "
		"without affecting previous settings.",
		m_preferencesPage));

	return true;
}

QString DataLoggerPlugin::description() { return "Use IIO raw and scale attributes to plot and save data"; }

QString DataLoggerPlugin::pkgName() { return DATALOGGER_PKG_NAME; }

#include "moc_dataloggerplugin.cpp"
