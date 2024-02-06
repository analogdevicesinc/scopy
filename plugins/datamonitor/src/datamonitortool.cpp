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

	//// add monitors
	addMonitorButton = new QPushButton(this);

	tool->addWidgetToTopContainerHelper(addMonitorButton, TTA_LEFT);

	connect(addMonitorButton, &QPushButton::clicked, this, [=]() {
		MonitorPlot *monitorPlot = new MonitorPlot(this);

		int controllerId = m_flexGridLayout->addQWidgetToList(monitorPlot);
		m_flexGridLayout->addWidget(controllerId);

	});

	// generate channel monitors

	DMM dmm;

	QList<DataMonitorModel *> dmmList = dmm.getDmmMonitors(ctx);

	dataAcquisitionManager = new DataAcquisitionManager(this);

	foreach(DataMonitorModel *monitor, dmmList) {
		dataAcquisitionManager->getDataMonitorMap()->insert(monitor->getName(), monitor);
	}

	Q_EMIT m_flexGridLayout->reqestLayoutUpdate();

	// TODO connect to UI
	///////// end add monitor

	DataMonitorStyleHelper::DataMonitorToolStyle(this);

	// generate channel monitors

	dataAcquisitionManager = new DataAcquisitionManager(this);

	// TODO get a list of DataMonitorModel for each device we want to add
	// ?? use de generateMonitor function outside the tool ? tool->generateMonitor( monitor data ) ;
	for(int i = 0; i < 4; i++) {
		DataMonitorModel *channelModel = new DataMonitorModel("dev0:test " + QString::number(i),
									  StyleHelper::getColor("CH" + QString::number(i)),
									  new UnitOfMeasurement("Volt", "V"));

		channelModel->setReadStrategy(new TestReadStrategy());

		dataAcquisitionManager->getDataMonitorMap()->insert(channelModel->getName(), channelModel);
	}
	for(int i = 0; i < 3; i++) {
		DataMonitorModel *channelModel = new DataMonitorModel("dev1:test " + QString::number(i + 4),
															  StyleHelper::getColor("CH" + QString::number(i)),
															  new UnitOfMeasurement("Volt", "V"));

		channelModel->setReadStrategy(new TestReadStrategy());

		dataAcquisitionManager->getDataMonitorMap()->insert(channelModel->getName(), channelModel);
	}
	Q_EMIT m_flexGridLayout->reqestLayoutUpdate();

	//TODO connect to UI

	dataAcquisitionManager->updateActiveMonitors(true, dataAcquisitionManager->getDataMonitorMap()->begin().key());

	connect(runBtn, &QPushButton::toggled, this, [=](){
		dataAcquisitionManager->readData();
	});

	//// add monitors
	addMonitorButton = new QPushButton(this);

	tool->addWidgetToTopContainerHelper(addMonitorButton, TTA_LEFT);

	connect(addMonitorButton, &QPushButton::clicked, this, [=]() {
		DataMonitorController *dataMonitorController = new DataMonitorController(this);



		int controllerId = m_flexGridLayout->addQWidgetToList(dataMonitorController->dataMonitorView());
		m_flexGridLayout->addWidget(controllerId);

	});

	///////// end add monitor

	DataMonitorStyleHelper::DataMonitorToolStyle(this);
}

RunBtn *DataMonitorTool::getRunButton() const { return runBtn; }

void DataMonitorTool::generateMonitor(DataMonitorModel *model, CollapsableMenuControlButton *channelManager)
{
	// Generate controller
}
