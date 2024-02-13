#include "datamonitortool.hpp"

#include <QBoxLayout>
#include <datamonitorstylehelper.hpp>
#include <flexgridlayout.hpp>
#include <hoverwidget.h>
#include <menucontrolbutton.h>

#include <stylehelper.h>
#include <verticalchannelmanager.h>

#include <datamonitormodel.hpp>
#include <testreadstrategy.hpp>

#include <QDebug>
#include <dmm.hpp>
#include <dmmreadstrategy.hpp>
#include <datamonitorcontroller.hpp>

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

	openLatMenuBtn = new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(tool->rightContainer()), false, this);

	settingsButton = new GearBtn(this);
	infoBtn = new InfoBtn(this);
	runBtn = new RunBtn(this);
	clearBtn = new QPushButton("Clear", this);

	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);

	tool->addWidgetToTopContainerMenuControlHelper(openLatMenuBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsButton, TTA_LEFT);

	tool->addWidgetToTopContainerHelper(runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(clearBtn, TTA_RIGHT);

	grp = static_cast<OpenLastMenuBtn *>(openLatMenuBtn)->getButtonGroup();
	grp->addButton(settingsButton);

	// channel monitors layout
	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidgetResizable(true);
	m_flexGridLayout = new FlexGridLayout(100, this);
	m_scrollArea->setWidget(m_flexGridLayout);
	tool->addWidgetToCentralContainerHelper(m_scrollArea);

	// generate channel monitors

	DMM dmm;

	QList<DataMonitorModel *> dmmList = dmm.getDmmMonitors(ctx);

	dataAcquisitionManager = new DataAcquisitionManager(this);
	connect(clearBtn, &QPushButton::clicked, dataAcquisitionManager, &DataAcquisitionManager::clearMonitorsData);

	foreach(DataMonitorModel *monitor, dmmList) {
		dataAcquisitionManager->getDataMonitorMap()->insert(monitor->getName(), monitor);
	}

	Q_EMIT m_flexGridLayout->reqestLayoutUpdate();

	// TODO connect to UI

	connect(runBtn, &QPushButton::toggled, this, [=]() { dataAcquisitionManager->readData(); });

	//// add monitors
	addMonitorButton = new QPushButton(this);

	tool->addWidgetToTopContainerHelper(addMonitorButton, TTA_LEFT);

	connect(addMonitorButton, &QPushButton::clicked, this, [=]() {
		DataMonitorController *dataMonitorController = new DataMonitorController(dataAcquisitionManager, this);

		int controllerId = m_flexGridLayout->addQWidgetToList(dataMonitorController->dataMonitorView());
		m_flexGridLayout->addWidget(controllerId);

		tool->rightStack()->add(QString::number(controllerId), dataMonitorController->dataMonitorSettings());

		tool->openRightContainerHelper(true);
		tool->requestMenu(QString::number(controllerId));

		// toggle active monitors
		connect(dataMonitorController->dataMonitorView(), &DataMonitorView::widgetClicked, this, [=]() {
			// TODO toggle menu for the montiro
			dataMonitorController->dataMonitorView()->toggleSelected();
		});
	});

	///////// end add monitor

	DataMonitorStyleHelper::DataMonitorToolStyle(this);
}

RunBtn *DataMonitorTool::getRunButton() const { return runBtn; }

void DataMonitorTool::generateMonitor(DataMonitorModel *model, CollapsableMenuControlButton *channelManager)
{
	// Generate controller
}
