#include "datamonitortool.hpp"
#include <datamonitortoolsettings.hpp>

#include <QBoxLayout>
#include <datamonitorstylehelper.hpp>
#include <flexgridlayout.hpp>
#include <hoverwidget.h>
#include <menucontrolbutton.h>

#include <stylehelper.h>
#include <verticalchannelmanager.h>

#include <QDate>
#include <datamonitormodel.hpp>
#include <testreadstrategy.hpp>

#include <QDebug>
#include <dmm.hpp>
#include <dmmreadstrategy.hpp>
#include <datamonitorcontroller.hpp>
#include <datamonitorutils.hpp>
#include <timemanager.hpp>
#include "dynamicWidget.h"
#include "logdatatofile.hpp"

using namespace scopy;
using namespace datamonitor;

DataMonitorTool::DataMonitorTool(iio_context *ctx, QWidget *parent)
	: ctx(ctx)
	, QWidget{parent}
{
	QHBoxLayout *lay = new QHBoxLayout(this);
	setLayout(lay);

	tool = new ToolTemplate(this);
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(true);
	tool->leftContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);

	tool->setRightContainerWidth(300);
	tool->centralContainer()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	tool->openLeftContainerHelper(false);
	tool->openRightContainerHelper(false);

	lay->addWidget(tool);

	openLastMenuBtn = new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(tool->rightContainer()), false, this);

	settingsButton = new GearBtn(this);
	infoBtn = new InfoBtn(this);
	runBtn = new RunBtn(this);
	clearBtn = new QPushButton("Clear", this);

	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);

	tool->addWidgetToTopContainerMenuControlHelper(openLastMenuBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsButton, TTA_LEFT);

	tool->addWidgetToTopContainerHelper(runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(clearBtn, TTA_RIGHT);

	grp = static_cast<OpenLastMenuBtn *>(openLastMenuBtn)->getButtonGroup();
	grp->addButton(settingsButton);

	// channel monitors layout
	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidgetResizable(true);
	m_flexGridLayout = new FlexGridLayout(100, this);
	m_scrollArea->setWidget(m_flexGridLayout);
	tool->addWidgetToCentralContainerHelper(m_scrollArea);

	// tool settings
	DataMonitorToolSettings *toolSettings = new DataMonitorToolSettings(this);
	tool->rightStack()->add(DataMonitorUtils::getToolSettingsId(), toolSettings);

	connect(settingsButton, &GearBtn::clicked, this, [=, this]() {
		tool->openRightContainerHelper(true);
		tool->requestMenu(DataMonitorUtils::getToolSettingsId());
	});

	auto &&timeTracker = TimeManager::GetInstance();
	timeTracker->setTimerInterval(DataMonitorUtils::getReadIntervalDefaul());

	connect(toolSettings, &DataMonitorToolSettings::readIntervalChanged, timeTracker,
		&TimeManager::setTimerInterval);

	// generate channel monitors

	DMM dmm;

	QList<DataMonitorModel *> dmmList = dmm.getDmmMonitors(ctx);

	dataAcquisitionManager = new DataAcquisitionManager(this);
	connect(clearBtn, &QPushButton::clicked, dataAcquisitionManager, &DataAcquisitionManager::clearMonitorsData);

	foreach(DataMonitorModel *monitor, dmmList) {
		dataAcquisitionManager->getDataMonitorMap()->insert(monitor->getName(), monitor);
	}

	Q_EMIT m_flexGridLayout->reqestLayoutUpdate();

	startTime = new QLabel();

	connect(timeTracker, &TimeManager::timeout, dataAcquisitionManager, &DataAcquisitionManager::readData);

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
	});

	connect(clearBtn, &QPushButton::clicked, this, [=, this]() {
		if(runBtn->isChecked()) {
			Q_EMIT runBtn->toggled(false);
		}
		if(!first) {
			resetStartTime();
			first = true;
		}
	});

	//// add monitors
	addMonitorButton = new AddBtn(this);

	tool->addWidgetToTopContainerHelper(addMonitorButton, TTA_LEFT);

	connect(addMonitorButton, &QPushButton::clicked, this, [=, this]() {
		DataMonitorController *dataMonitorController = new DataMonitorController(dataAcquisitionManager, this);

		int controllerId = m_flexGridLayout->addQWidgetToList(dataMonitorController->dataMonitorView());
		m_flexGridLayout->addWidget(controllerId);

		tool->rightStack()->add(QString::number(controllerId), dataMonitorController->dataMonitorSettings());

		// toggle active monitors
		connect(dataMonitorController->dataMonitorView(), &DataMonitorView::widgetClicked, this, [=, this]() {
			tool->openRightContainerHelper(true);
			tool->requestMenu(QString::number(controllerId));
			// TODO toggle menu for the montiro
			dataMonitorController->dataMonitorView()->toggleSelected();
			// TODO handle active monitor
		});

		// remove the monitor
		connect(dataMonitorController->dataMonitorView(), &DataMonitorView::removeMonitor, [=, this]() {
			m_flexGridLayout->removeWidget(controllerId);
			tool->rightStack()->remove(QString::number(controllerId));
			tool->openRightContainerHelper(false);
			delete dataMonitorController;
		});

		// updatePlotRealTime
		connect(this, &DataMonitorTool::setStartTime, dataMonitorController->dataMonitorView()->monitorPlot(),
			&MonitorPlot::setStartTime);
	});

	///////// end add monitor

	tool->addWidgetToTopContainerHelper(startTime, TTA_LEFT);

	Q_EMIT addMonitorButton->clicked();

	/// log data
	LogDataToFile *logDataToFile = new LogDataToFile(dataAcquisitionManager, this);

	connect(toolSettings, &DataMonitorToolSettings::requestDataLogging, logDataToFile, &LogDataToFile::logData);
	///

	DataMonitorStyleHelper::DataMonitorToolStyle(this);
}

RunBtn *DataMonitorTool::getRunButton() const { return runBtn; }

void DataMonitorTool::generateMonitor(DataMonitorModel *model, CollapsableMenuControlButton *channelManager)
{
	// Generate controller
}

void DataMonitorTool::resetStartTime()
{
	auto &&timeTracker = TimeManager::GetInstance();
	timeTracker->setStartTime();

	QDateTime date = QDateTime::currentDateTime();
	QString formattedTime = date.toString("dd/MM/yyyy hh:mm:ss");
	QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
	startTime->setText(QString(formattedTimeMsg));
	Q_EMIT setStartTime();
}
