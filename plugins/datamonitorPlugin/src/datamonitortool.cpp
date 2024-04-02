#include "datamonitortool.h"

#include <QBoxLayout>
#include <QDateTime>
#include <datamonitorutils.hpp>
#include <logdatatofile.hpp>
#include <menucontrolbutton.h>
#include <sevensegmentdisplay.hpp>
#include <timemanager.hpp>
#include "datamonitorstylehelper.hpp"
#include <iioutil/connection.h>

using namespace scopy::datamonitor;

Q_LOGGING_CATEGORY(CAT_DATAMONITOR, "DataMonitorPlugin");
Q_LOGGING_CATEGORY(CAT_DATAMONITOR_TOOL, "DataMonitorTool");

DatamonitorTool::DatamonitorTool(DataAcquisitionManager *dataAcquisitionManager, QWidget *parent)
	: m_dataAcquisitionManager(dataAcquisitionManager)
	, QWidget{parent}
{
	QHBoxLayout *lay = new QHBoxLayout(this);
	setLayout(lay);

	tool = new ToolTemplate(this);
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(true);
	tool->leftContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	tool->bottomContainer()->setVisible(true);
	tool->topContainerMenuControl()->setVisible(false);

	tool->openLeftContainerHelper(false);
	tool->openRightContainerHelper(false);

	lay->addWidget(tool);

	settingsButton = new GearBtn(this);
	infoBtn = new InfoBtn(this);
	printBtn = new PrintBtn(this);
	runBtn = new RunBtn(this);
	clearBtn = new QPushButton("Clear", this);

	//// add monitors
	addMonitorButton = new AddBtn(this);

	connect(addMonitorButton, &AddBtn::clicked, this, &DatamonitorTool::requestNewTool);

	monitorsButton = new MenuControlButton(this);
	monitorsButton->setName("Monitors");
	monitorsButton->setOpenMenuChecksThis(true);
	monitorsButton->setDoubleClickToOpenMenu(false);
	monitorsButton->checkBox()->setVisible(false);
	monitorsButton->button()->setVisible(false);

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

	tool->addWidgetToTopContainerHelper(settingsButton, TTA_RIGHT);

	tool->addWidgetToTopContainerHelper(addMonitorButton, TTA_LEFT);

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
	m_monitorPlot = new MonitorPlot(this);
	centralWidget->addWidget(m_monitorPlot);

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

	SevenSegmentDisplay *sevenSegmetMonitors = new SevenSegmentDisplay(this);
	centralWidget->addWidget(sevenSegmetMonitors);

	////////////////////////settings //////////////
	m_dataMonitorSettings = new DataMonitorSettings(m_monitorPlot);
	// TODO GET SETTINGS NAME FROM UTILS
	m_dataMonitorSettings->init("DataMonitor", StyleHelper::getColor("ScopyBlue"));

	tool->rightStack()->add(DataMonitorUtils::getToolSettingsId(), m_dataMonitorSettings);

	connect(m_dataMonitorSettings, &DataMonitorSettings::titleUpdated, this,
		&DatamonitorTool::settingsTitleChanged);
	connect(settingsButton, &GearBtn::toggled, this, [=, this](bool toggled) {
		tool->openRightContainerHelper(toggled);
		tool->requestMenu(DataMonitorUtils::getToolSettingsId());
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

	MenuControlButton *showPlot = new MenuControlButton(this);
	showPlot->setName("Plot");
	showPlot->setOpenMenuChecksThis(true);
	showPlot->setDoubleClickToOpenMenu(true);
	showPlot->checkBox()->setVisible(false);
	showPlot->button()->setVisible(false);
	showPlot->setChecked(true);

	connect(showPlot, &QPushButton::clicked, this, [=, this]() { centralWidget->setCurrentWidget(m_monitorPlot); });

	MenuControlButton *showText = new MenuControlButton(this);
	showText->setName("Text");
	showText->setOpenMenuChecksThis(true);
	showText->setDoubleClickToOpenMenu(true);
	showText->checkBox()->setVisible(false);
	showText->button()->setVisible(false);
	connect(showText, &QPushButton::clicked, this, [=, this]() { centralWidget->setCurrentWidget(textMonitors); });

	MenuControlButton *showSegments = new MenuControlButton(this);
	showSegments->setName("7 Segment");
	showSegments->setOpenMenuChecksThis(true);
	showSegments->setDoubleClickToOpenMenu(true);
	showSegments->checkBox()->setVisible(false);
	showSegments->button()->setVisible(false);
	connect(showSegments, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(sevenSegmetMonitors); });

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

	connect(m_dataMonitorSettings->getDataLoggingMenu(), &DataLoggingMenu::requestDataLoading, logDataToFile,
		&LogDataToFile::loadData);

	/////////////////monitor selection menu ///////////////

	m_monitorSelectionMenu = new MonitorSelectionMenu(dataAcquisitionManager->getDataMonitorMap());
	tool->leftStack()->add("Monitors", m_monitorSelectionMenu);

	connect(m_dataAcquisitionManager, &DataAcquisitionManager::monitorAdded, m_monitorSelectionMenu,
		&MonitorSelectionMenu::addMonitor);

	connect(m_dataAcquisitionManager, &DataAcquisitionManager::monitorRemoved, this,
		[=, this](QString monitorName) {
			m_monitorPlot->removeMonitor(monitorName);
			sevenSegmetMonitors->removeSegment(monitorName);
		});

	connect(m_dataAcquisitionManager, &DataAcquisitionManager::deviceRemoved, m_monitorSelectionMenu,
		&MonitorSelectionMenu::removeDevice);

	connect(m_monitorSelectionMenu, &MonitorSelectionMenu::monitorToggled, m_monitorPlot,
		[=, this](bool toggled, QString monitorName) {
			// toggle monitor active inside data acquisiton manager
			m_dataAcquisitionManager->updateActiveMonitors(toggled, monitorName);

			// handle monitor on plot
			if(toggled) {
				m_monitorPlot->addMonitor(
					m_dataAcquisitionManager->getDataMonitorMap()->value(monitorName));

				sevenSegmetMonitors->generateSegment(
					m_dataAcquisitionManager->getDataMonitorMap()->value(monitorName));

			} else {
				m_monitorPlot->removeMonitor(monitorName);
				sevenSegmetMonitors->removeSegment(monitorName);
			}
		});

	connect(m_monitorSelectionMenu, &MonitorSelectionMenu::requestRemoveImportedDevice, m_dataAcquisitionManager,
		&DataAcquisitionManager::removeDevice);

	DataMonitorStyleHelper::DataMonitorToolStyle(this);
}

DatamonitorTool::~DatamonitorTool() {}

scopy::RunBtn *DatamonitorTool::getRunButton() const { return runBtn; }

void DatamonitorTool::resetStartTime()
{
	auto &&timeTracker = TimeManager::GetInstance();
	timeTracker->setStartTime();
	m_monitorPlot->setStartTime();
}
