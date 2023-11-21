#include "datamonitortool.hpp"

#include <QBoxLayout>
#include <flexgridlayout.hpp>
#include <hoverwidget.h>
#include <menucontrolbutton.h>
#include <stylehelper.h>
#include <verticalchannelmanager.h>

#include <src/datamonitor/datamonitorcontroller.hpp>
#include <src/datamonitor/datamonitormodel.hpp>

using namespace scopy;
using namespace datamonitor;

DataMonitorTool::DataMonitorTool(QWidget *parent)
	: QWidget{parent}
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
	singleBtn = new SingleShotBtn(this);

	tool->addWidgetToTopContainerMenuControlHelper(openLatMenuBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsButton, TTA_LEFT);

	tool->addWidgetToTopContainerHelper(runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(singleBtn, TTA_RIGHT);

	grp = static_cast<OpenLastMenuBtn *>(openLatMenuBtn)->getButtonGroup();
	grp->addButton(settingsButton);

	// channel monitors layout
	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidgetResizable(true);
	m_flexGridLayout = new FlexGridLayout(100, this);
	m_scrollArea->setWidget(m_flexGridLayout);
	tool->addWidgetToCentralContainerHelper(m_scrollArea);

	initDataMonitor();
}

RunBtn *DataMonitorTool::getRunButton() const { return runBtn; }

void DataMonitorTool::initDataMonitor()
{
	// CHANNEL MANAGER
	MenuControlButton *channels = new MenuControlButton(this);
	channels->setCheckBoxStyle(MenuControlButton::CS_COLLAPSE);
	channels->setName("Channels");
	channels->button()->setVisible(false);
	channels->checkBox()->setChecked(true);

	tool->addWidgetToTopContainerHelper(channels, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);

	VerticalChannelManager *vcm = new VerticalChannelManager(tool->leftContainer());
	vcm->setFixedWidth(145);

	// create one for each device
	CollapsableMenuControlButton *channelManager = new CollapsableMenuControlButton(this);
	channelManager->getControlBtn()->setName("Dev");
	vcm->add(channelManager);

	HoverWidget *hv = new HoverWidget(nullptr, channels, tool);
	hv->setContent(vcm);
	hv->setAnchorOffset(QPoint(0, 10));
	hv->setAnchorPos(HoverPosition::HP_BOTTOMLEFT);
	hv->setContentPos(HoverPosition::HP_BOTTOMRIGHT);

	hv->setVisible(true);
	hv->raise();

	connect(channels->checkBox(), &QCheckBox::toggled, this, [=](bool toggled) {
		hv->setVisible(toggled);
		hv->raise();
	});

	// generate channel monitors
	m_monitors = new QMap<int, DataMonitorController *>();

	// TODO get a list of DataMonitorModel for each device we want to add
	// ?? use de generateMonitor function outside the tool ? tool->generateMonitor( monitor data ) ;
	for(int i = 0; i < 4; i++) {
		DataMonitorModel *channelModel = new DataMonitorModel("test:" + QString::number(i),
								      StyleHelper::getColor("CH" + QString::number(i)),
								      new UnitOfMeasurement("Volt", "V"));

		generateMonitor(channelModel, channelManager);
	}
	Q_EMIT m_flexGridLayout->reqestLayoutUpdate();
}

void DataMonitorTool::generateMonitor(DataMonitorModel *model, CollapsableMenuControlButton *channelManager)
{
	// Generate controller
	DataMonitorController *dataMonitorController = new DataMonitorController(model);
	int controllerId = m_flexGridLayout->addQWidgetToList(dataMonitorController->getDataMonitorView());
	m_monitors->insert(controllerId, dataMonitorController);
	// add the monitor to layout
	m_flexGridLayout->addWidget(controllerId);
}

void DataMonitorTool::initDataMonitor() {}
