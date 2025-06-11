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

#include "menus/channelattributesmenu.hpp"
#include "datamonitortool.h"
#include "monitorplotmanager.h"

#include <QBoxLayout>
#include <QDateTime>
#include <datamonitorutils.hpp>
#include <datamonitor/dmmdatamonitormodel.hpp>
#include <menus/logdatatofile.hpp>
#include <menucontrolbutton.h>
#include <datamonitor/sevensegmentdisplay.hpp>
#include <QDesktopServices>
#include <style.h>
#include <timemanager.hpp>
#include <tutorialbuilder.h>
#include <pluginbase/preferences.h>
#include "datamonitorstylehelper.hpp"
#include <iioutil/connection.h>
#include <QComboBox>

using namespace scopy::datamonitor;

Q_LOGGING_CATEGORY(CAT_DATAMONITOR, "DataLoggerPlugin");
Q_LOGGING_CATEGORY(CAT_DATAMONITOR_TOOL, "DataMonitorTool");

DatamonitorTool::DatamonitorTool(DataAcquisitionManager *dataAcquisitionManager, bool isDeletable, QWidget *parent)
	: m_dataAcquisitionManager(dataAcquisitionManager)
	, QWidget{parent}
{
	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setMargin(0);
	setLayout(lay);

	tool = new ToolTemplate(this);
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(true);
	tool->leftContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	tool->bottomContainer()->setVisible(true);

	lay->addWidget(tool);

	settingsButton = new GearBtn(this);
	settingsButton->setChecked(true);
	openLastMenuBtn = new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(tool->rightContainer()), true, this);
	rightMenuBtnGrp = dynamic_cast<OpenLastMenuBtn *>(openLastMenuBtn)->getButtonGroup();

	printplotManager = new PrintPlotManager(this);
	runBtn = new RunBtn(this);
	clearBtn = new QPushButton("Clear", this);
	PrintBtn *printBtn = new PrintBtn(this);

	infoBtn = new InfoBtn(this, true);

	connect(infoBtn, &InfoBtn::clicked, this, [this]() {
		infoBtn->generateInfoPopup(this);

		connect(infoBtn->getTutorialButton(), &QPushButton::clicked, this, &DatamonitorTool::startTutorial);
		connect(infoBtn->getDocumentationButton(), &QPushButton::clicked, this, [=]() {
			QDesktopServices::openUrl(
				QUrl("https://analogdevicesinc.github.io/scopy/plugins/datalogger/datalogger.html"));
		});
	});

	//// add monitors
	addMonitorToolButton = new AddBtn(this);

	connect(addMonitorToolButton, &AddBtn::clicked, this, &DatamonitorTool::requestNewTool);

	Preferences *p = Preferences::GetInstance();
	QObject::connect(p, &Preferences::preferenceChanged, this, [=, this](QString id, QVariant var) {
		if(id.contains("dataloggerplugin_add_remove_instrument")) {
			bool en = p->get("dataloggerplugin_add_remove_instrument").toDouble();
			setEnableAddRemoveInstrument(en);
		}
	});

	removeBtn = new RemoveBtn(this);
	if(!isDeletable) {
		removeBtn->setVisible(false);
	}
	connect(removeBtn, &AddBtn::clicked, this, &DatamonitorTool::requestDeleteTool);

	monitorsButton = new MenuControlButton(this);
	monitorsButton->setName("Monitors");
	monitorsButton->setOpenMenuChecksThis(true);
	monitorsButton->setDoubleClickToOpenMenu(false);
	monitorsButton->checkBox()->setVisible(false);
	monitorsButton->button()->setVisible(false);
	monitorsButton->setChecked(true);

	tool->addWidgetToBottomContainerHelper(monitorsButton, TTA_LEFT);

	connect(monitorsButton, &GearBtn::toggled, this, [=, this](bool toggled) {
		tool->openLeftContainerHelper(toggled);
		tool->requestMenu("Monitors");
	});

	//////////////////////////////

	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(printBtn, TTA_LEFT);

	tool->addWidgetToTopContainerHelper(runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(clearBtn, TTA_RIGHT);

	tool->addWidgetToTopContainerMenuControlHelper(openLastMenuBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsButton, TTA_LEFT);

	tool->addWidgetToTopContainerHelper(addMonitorToolButton, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(removeBtn, TTA_LEFT);

	///// time manager
	auto &&timeTracker = TimeManager::GetInstance();

	connect(runBtn, &QPushButton::toggled, this, [=, this](bool toggled) {
		if(toggled) {
			timeTracker->startTimer();
			if(first) {
				resetStartTime();
				first = false;
			}
		} else {
			timeTracker->stopTimer();
		}

		timeTracker->setIsRunning(toggled);
		Q_EMIT runToggled(toggled);
	});

	connect(clearBtn, &QPushButton::clicked, dataAcquisitionManager, &DataAcquisitionManager::clearMonitorsData);
	connect(clearBtn, &QPushButton::clicked, this, [=, this]() { resetStartTime(); });

	////////central section ////
	QStackedWidget *centralWidget = new QStackedWidget(this);
	tool->addWidgetToCentralContainerHelper(centralWidget);

	/////////////////////////plot///////////////////
	m_plotManager = new MonitorPlotManager("MonitorPlotManager", this);
	centralWidget->addWidget(m_plotManager);

	// create the first plot
	uint32_t plotId = m_plotManager->addPlot("Monitor Plot");

	connect(printBtn, &QPushButton::clicked, this, [=, this]() {
		QList<PlotWidget *> plotList;

		for(PlotComponent *pp : m_plotManager->plots()) {
			for(PlotWidget *plt : pp->plots()) {
				plotList.push_back(plt);
			}
		}
		printplotManager->printPlots(plotList, "Data Logger");
	});

	/////////////////////text values ////////////
	textMonitors = new QTextEdit(this);
	textMonitors->setReadOnly(true);

	centralWidget->addWidget(textMonitors);

	connect(timeTracker, &TimeManager::timeout, this, [=, this]() {
		textMonitors->clear();
		foreach(QString monitorName, dataAcquisitionManager->getActiveMonitors()) {
			auto *monitor = dataAcquisitionManager->getDataMonitorMap()->value(monitorName);
			textMonitors->append(monitor->getName() + ":  " +
					     QString::number(monitor->getLastReadValue().second) + "  " +
					     monitor->getUnitOfMeasure()->getNameAndSymbol());
		}
	});

	//////// 7 segment widget ////

	sevenSegmetMonitors = new SevenSegmentDisplay(this);
	centralWidget->addWidget(sevenSegmetMonitors);

	////////////////////////settings //////////////
	m_dataMonitorSettings = new DataMonitorSettings(m_plotManager);

	m_dataMonitorSettings->init("Data Logger", Style::getAttribute(json::theme::interactive_primary_idle));
	tool->rightStack()->add(DataMonitorUtils::getToolSettingsId(), m_dataMonitorSettings);

	connect(m_dataMonitorSettings, &DataMonitorSettings::titleUpdated, this,
		&DatamonitorTool::settingsTitleChanged);
	connect(settingsButton, &GearBtn::toggled, this, [=, this](bool toggled) {
		if(toggled) {
			tool->requestMenu(DataMonitorUtils::getToolSettingsId());
		}
	});

	/// 7 segments settings ///
	connect(m_dataMonitorSettings->getSevenSegmentMonitorSettings(), &SevenSegmentMonitorSettings::precisionChanged,
		sevenSegmetMonitors, &SevenSegmentDisplay::updatePrecision);
	connect(m_dataMonitorSettings->getSevenSegmentMonitorSettings(),
		&SevenSegmentMonitorSettings::peakHolderToggled, sevenSegmetMonitors,
		&SevenSegmentDisplay::togglePeakHolder);

	////togle between plot and live values

	QButtonGroup *centralWidgetButtons = new QButtonGroup(this);
	centralWidgetButtons->setExclusive(true);

	showPlot = new MenuControlButton(this);
	showPlot->setName("Plot");
	showPlot->setOpenMenuChecksThis(true);
	showPlot->setDoubleClickToOpenMenu(true);
	showPlot->checkBox()->setVisible(false);
	showPlot->button()->setVisible(false);
	showPlot->setChecked(true);

	connect(showPlot, &QPushButton::clicked, this, [=, this]() {
		centralWidget->setCurrentWidget(m_plotManager);
		m_dataMonitorSettings->setActiveSettings(0);
	});

	showText = new MenuControlButton(this);
	showText->setName("Text");
	showText->setOpenMenuChecksThis(true);
	showText->setDoubleClickToOpenMenu(true);
	showText->checkBox()->setVisible(false);
	showText->button()->setVisible(false);
	connect(showText, &QPushButton::clicked, this, [=, this]() {
		centralWidget->setCurrentWidget(textMonitors);
		// display only the data logging settings for this view
		m_dataMonitorSettings->setActiveSettings(2);
	});

	showSegments = new MenuControlButton(this);
	showSegments->setName("7 Segment");
	showSegments->setOpenMenuChecksThis(true);
	showSegments->setDoubleClickToOpenMenu(true);
	showSegments->checkBox()->setVisible(false);
	showSegments->button()->setVisible(false);
	connect(showSegments, &QPushButton::clicked, this, [=, this]() {
		centralWidget->setCurrentWidget(sevenSegmetMonitors);
		m_dataMonitorSettings->setActiveSettings(1);
	});

	centralWidgetButtons->addButton(showPlot);
	centralWidgetButtons->addButton(showText);
	centralWidgetButtons->addButton(showSegments);

	tool->addWidgetToBottomContainerHelper(showPlot, TTA_RIGHT);
	tool->addWidgetToBottomContainerHelper(showText, TTA_RIGHT);
	tool->addWidgetToBottomContainerHelper(showSegments, TTA_RIGHT);

	/// log data
	LogDataToFile *logDataToFile = new LogDataToFile(dataAcquisitionManager, this);

	connect(m_dataMonitorSettings->getDataLoggingMenu(), &DataLoggingMenu::requestLiveDataLogging, logDataToFile,
		&LogDataToFile::continuousLogData);

	connect(m_dataMonitorSettings->getDataLoggingMenu(), &DataLoggingMenu::requestDataLogging, logDataToFile,
		&LogDataToFile::logData);

	connect(logDataToFile, &LogDataToFile::loadDataCompleted, m_dataMonitorSettings->getDataLoggingMenu(),
		[=, this]() {
			m_dataMonitorSettings->getDataLoggingMenu()->updateDataLoggingStatus(ProgressBarState::SUCCESS);
		});

	connect(logDataToFile, &LogDataToFile::logDataCompleted, m_dataMonitorSettings->getDataLoggingMenu(),
		[=, this]() {
			m_dataMonitorSettings->getDataLoggingMenu()->updateDataLoggingStatus(ProgressBarState::SUCCESS);
		});

	connect(logDataToFile, &LogDataToFile::logDataError, m_dataMonitorSettings->getDataLoggingMenu(), [=, this]() {
		m_dataMonitorSettings->getDataLoggingMenu()->updateDataLoggingStatus(ProgressBarState::ERROR);
	});

	connect(m_dataMonitorSettings->getDataLoggingMenu(), &DataLoggingMenu::requestDataLoading, logDataToFile,
		&LogDataToFile::loadData);

	////generate channel settings for compatible monitors
	foreach(QString monitor, m_dataAcquisitionManager->getDataMonitorMap()->keys()) {
		auto monitorModel = m_dataAcquisitionManager->getDataMonitorMap()->value(monitor);
		tool->rightStack()->add(monitor, new ChannelAttributesMenu(monitorModel, m_plotManager, this));
	}

	/////////////////monitor selection menu ///////////////
	m_monitorSelectionMenu = new MonitorSelectionMenu(dataAcquisitionManager->getDataMonitorMap(), rightMenuBtnGrp);

	rightMenuBtnGrp->addButton(settingsButton);

	tool->leftStack()->add("Monitors", m_monitorSelectionMenu);

	connect(m_dataAcquisitionManager, &DataAcquisitionManager::monitorAdded, this,
		[=, this](DataMonitorModel *monitor) {
			m_monitorSelectionMenu->addMonitor(monitor);

			tool->rightStack()->add(monitor->getName(),
						new ChannelAttributesMenu(monitor, m_plotManager, this));
		});

	connect(m_dataAcquisitionManager, &DataAcquisitionManager::monitorRemoved, this,
		[=, this](QString monitorName) {
			m_plotManager->removePlotCurve(
				m_dataAcquisitionManager->getDataMonitorMap()->value(monitorName));
			sevenSegmetMonitors->removeSegment(monitorName);
			tool->rightStack()->remove(monitorName);
		});

	connect(m_dataAcquisitionManager, &DataAcquisitionManager::deviceRemoved, m_monitorSelectionMenu,
		&MonitorSelectionMenu::removeDevice);

	connect(m_monitorSelectionMenu, &MonitorSelectionMenu::monitorToggled, this,
		[=, this](bool toggled, QString monitorName) {
			// Only update active monitor list, not plot assignment
			m_dataAcquisitionManager->updateActiveMonitors(toggled, monitorName);
			// handle monitor on plot
			if(toggled) {
				m_plotManager->addPlotCurve(
					m_dataAcquisitionManager->getDataMonitorMap()->value(monitorName));
				sevenSegmetMonitors->generateSegment(
					m_dataAcquisitionManager->getDataMonitorMap()->value(monitorName));

			} else {
				m_plotManager->removePlotCurve(
					m_dataAcquisitionManager->getDataMonitorMap()->value(monitorName));
				sevenSegmetMonitors->removeSegment(monitorName);
			}
		});

	connect(m_monitorSelectionMenu, &MonitorSelectionMenu::requestRemoveImportedDevice, m_dataAcquisitionManager,
		&DataAcquisitionManager::removeDevice);

	connect(m_monitorSelectionMenu, &MonitorSelectionMenu::requestMonitorMenu, this,
		[=, this](bool toggled, QString monitor) {
			if(toggled) {
				tool->requestMenu(monitor);
			}
		});

	initTutorialProperties();
	DataMonitorStyleHelper::DataMonitorToolStyle(this);
}

DatamonitorTool::~DatamonitorTool() {}

scopy::RunBtn *DatamonitorTool::getRunButton() const { return runBtn; }

void DatamonitorTool::resetStartTime()
{
	auto &&timeTracker = TimeManager::GetInstance();
	timeTracker->setStartTime();
	Q_EMIT m_plotManager->requestSetStartTime();
}

void DatamonitorTool::initTutorialProperties()
{
	runBtn->setProperty("tutorial_name", "RUN_BUTTON");
	clearBtn->setProperty("tutorial_name", "CLEAR_BUTTON");

	if(addMonitorToolButton->isVisible()) {
		addMonitorToolButton->setProperty("tutorial_name", "ADD_BUTTON");
	}

	showPlot->setProperty("tutorial_name", "SHOW_PLOT_BUTTON");
	showText->setProperty("tutorial_name", "SHOW_TEXT_BUTTON");
	showSegments->setProperty("tutorial_name", "SHOW_7SEG_BUTTON");
	settingsButton->setProperty("tutorial_name", "SETTINGS_BUTTON");
	monitorsButton->setProperty("tutorial_name", "MONITORS_MENU_BUTTON");
}

void DatamonitorTool::startTutorial()
{
	QWidget *parent = Util::findContainingWindow(this);
	gui::TutorialBuilder *datamonitorTutorial =
		new gui::TutorialBuilder(this, ":/datamonitor/tutorial_chapters.json", "datamonitor", parent);
	datamonitorTutorial->setTitle("Tutorial");
	datamonitorTutorial->start();
}

void DatamonitorTool::setEnableAddRemovePlot(bool en) { m_dataMonitorSettings->setEnableAddRemovePlot(en); }

void DatamonitorTool::setEnableAddRemoveInstrument(bool en) { addMonitorToolButton->setVisible(en); }

#include "moc_datamonitortool.cpp"
