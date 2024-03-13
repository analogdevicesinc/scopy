#include "adcinstrument.h"

#include "gui/widgets/measurementsettings.h"

#include <gui/widgets/hoverwidget.h>
#include <gui/widgets/measurementpanel.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/toolbuttons.h>
#include <gui/widgets/verticalchannelmanager.h>

using namespace scopy;
using namespace scopy::grutil;

AdcInstrument::AdcInstrument(PlotRecipe *r,QWidget *parent)
	: QWidget(parent)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setMargin(0);
	setLayout(lay);
	tool = new ToolTemplate(this);
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->bottomContainer()->setVisible(true);
	tool->topContainer()->setVisible(true);
	tool->leftContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	tool->topCentral()->setVisible(true);
	tool->bottomCentral()->setVisible(false);
	lay->addWidget(tool);
	tool->setLeftContainerWidth(210);
	tool->setRightContainerWidth(300);
	tool->setTopContainerHeight(100);
	tool->setBottomContainerHeight(90);

	openLastMenuBtn = new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(tool->rightContainer()), true, this);
	rightMenuBtnGrp = dynamic_cast<OpenLastMenuBtn *>(openLastMenuBtn)->getButtonGroup();

	tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);

	GearBtn *settingsBtn = new GearBtn(this);
	InfoBtn *infoBtn = new InfoBtn(this);
	PrintBtn *printBtn = new PrintBtn(this);

	AddBtn *addPlotBtn = new AddBtn(this);
	RemoveBtn *removePlotBtn = new RemoveBtn(this);
	SyncBtn *syncPlotBtn = new SyncBtn(this);

	connect(addPlotBtn,&QPushButton::clicked,this,[=](){
		Q_EMIT newPlot(0);});
	connect(removePlotBtn,&QPushButton::clicked,this,[=](){Q_EMIT removePlot(this);});


	runBtn = new RunBtn(this);
	singleBtn = new SingleShotBtn(this);

	channelsBtn = new MenuControlButton(this);
	setupChannelsButtonHelper(channelsBtn);

	MenuControlButton *cursor = new MenuControlButton(this);
	setupCursorButtonHelper(cursor);

	MenuControlButton *measure = new MenuControlButton(this);
	setupMeasureButtonHelper(measure);
	measure_panel = new MeasurementsPanel(this);
	tool->topStack()->add(measureMenuId, measure_panel);

	stats_panel = new StatsPanel(this);
	tool->bottomStack()->add(statsMenuId, stats_panel);

	measureSettings = new MeasurementSettings(this);
	HoverWidget *measurePanelManagerHover = new HoverWidget(nullptr, measure, tool);
	measurePanelManagerHover->setContent(measureSettings);
	measurePanelManagerHover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	measurePanelManagerHover->setContentPos(HoverPosition::HP_TOPLEFT);
	connect(measure->button(), &QPushButton::toggled, this, [=](bool b) {
		measurePanelManagerHover->setVisible(b);
		measurePanelManagerHover->raise();
	});
	connect(measureSettings, &MeasurementSettings::enableMeasurementPanel, tool->topCentral(),
		&QWidget::setVisible);
	connect(measureSettings, &MeasurementSettings::enableStatsPanel, tool->bottomCentral(), &QWidget::setVisible);

	connect(measureSettings, &MeasurementSettings::sortMeasurements, measure_panel, &MeasurementsPanel::sort);
	connect(measureSettings, &MeasurementSettings::sortStats, stats_panel, &StatsPanel::sort);

	tool->addWidgetToTopContainerMenuControlHelper(openLastMenuBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsBtn, TTA_LEFT);

	tool->addWidgetToTopContainerHelper(runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(singleBtn, TTA_RIGHT);

	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(printBtn, TTA_LEFT);

	tool->addWidgetToTopContainerHelper(addPlotBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(removePlotBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(syncPlotBtn, TTA_LEFT);



	tool->addWidgetToBottomContainerHelper(channelsBtn, TTA_LEFT);
	//	tool->addWidgetToBottomContainerHelper(timeBtn, TTA_LEFT);
	//	tool->addWidgetToBottomContainerHelper(xyBtn, TTA_LEFT);
	//	tool->addWidgetToBottomContainerHelper(fftBtn, TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(cursor, TTA_RIGHT);
	tool->addWidgetToBottomContainerHelper(measure, TTA_RIGHT);

	connect(channelsBtn, &QPushButton::toggled, dynamic_cast<MenuHAnim *>(tool->leftContainer()),
		&MenuHAnim::toggleMenu);

	vcm = new VerticalChannelManager(this);
	tool->leftStack()->add(verticalChannelManagerId, vcm);

	connect(runBtn, &QPushButton::toggled, this, &AdcInstrument::setRunning);
	connect(singleBtn, &QPushButton::toggled, this, &AdcInstrument::setRunning);
	connect(this, &AdcInstrument::runningChanged, this, &AdcInstrument::run);
	connect(this, &AdcInstrument::runningChanged, runBtn, &QAbstractButton::setChecked);

	connect(measure, &MenuControlButton::toggled, this, &AdcInstrument::showMeasurements);

	m_recipe = r;
	m_recipe->setAdcInstrument(this);
	init();
}

void AdcInstrument::init() {
	m_recipe->init();
}

void AdcInstrument::deinit() {
	m_recipe->deinit();
}

void AdcInstrument::initCursors()
{
	cursorController->getPlotCursors()->getH1Cursor()->setPosition(0);
	cursorController->getPlotCursors()->getH2Cursor()->setPosition(0);
	cursorController->getPlotCursors()->getV1Cursor()->setPosition(0);
	cursorController->getPlotCursors()->getV2Cursor()->setPosition(0);
}

AdcInstrument::~AdcInstrument() {
	deinit();
}

bool AdcInstrument::running() const
{
	return m_running;
}

void AdcInstrument::setRunning(bool r)
{
	m_running = r;
}


void AdcInstrument::setupChannelMeasurement(ChannelAddon *ch)
{
	auto chMeasureableChannel = dynamic_cast<MeasurementProvider *>(ch);
	if(!chMeasureableChannel)
		return;
	auto chMeasureManager = chMeasureableChannel->getMeasureManager();
	if(!chMeasureManager)
		return;
	if(measureSettings) {
		connect(chMeasureManager, &MeasureManagerInterface::enableMeasurement, measure_panel,
			&MeasurementsPanel::addMeasurement);
		connect(chMeasureManager, &MeasureManagerInterface::disableMeasurement, measure_panel,
			&MeasurementsPanel::removeMeasurement);
		connect(measureSettings, &MeasurementSettings::toggleAllMeasurements, chMeasureManager,
			&MeasureManagerInterface::toggleAllMeasurement);
		connect(measureSettings, &MeasurementSettings::toggleAllStats, chMeasureManager,
			&MeasureManagerInterface::toggleAllStats);
		connect(chMeasureManager, &MeasureManagerInterface::enableStat, stats_panel, &StatsPanel::addStat);
		connect(chMeasureManager, &MeasureManagerInterface::disableStat, stats_panel, &StatsPanel::removeStat);
	}
}

void AdcInstrument::setupTimeButtonHelper(MenuControlButton *time)
{
	time->setName("Time");
	time->setOpenMenuChecksThis(true);
	time->setDoubleClickToOpenMenu(true);
	time->checkBox()->setVisible(false);
	time->setCheckBoxStyle(MenuControlButton::CS_SQUARE);
}

void AdcInstrument::setupCursorButtonHelper(MenuControlButton *cursor)
{
	cursor->setName("Cursors");
	cursor->setOpenMenuChecksThis(true);
	cursor->setDoubleClickToOpenMenu(true);
	cursor->checkBox()->setVisible(false);
	cursor->setCheckBoxStyle(MenuControlButton::CS_SQUARE);
}

void AdcInstrument::setupMeasureButtonHelper(MenuControlButton *btn)
{
	btn->setName("Measure");
	btn->setOpenMenuChecksThis(true);
	btn->setDoubleClickToOpenMenu(true);
	btn->checkBox()->setVisible(false);
}

void AdcInstrument::setupDeviceMenuControlButtonHelper(MenuControlButton *devBtn, GRDeviceAddon *dev)
{
	devBtn->setName(dev->getName());
	devBtn->setCheckable(true);
	devBtn->button()->setVisible(false);
	devBtn->setOpenMenuChecksThis(true);
	devBtn->setDoubleClickToOpenMenu(true);
}

void AdcInstrument::setupChannelMenuControlButtonHelper(MenuControlButton *btn, ChannelAddon *ch)
{
	btn->setName(ch->getName());
	btn->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	btn->setOpenMenuChecksThis(true);
	btn->setDoubleClickToOpenMenu(true);
	btn->setColor(ch->pen().color());
	btn->button()->setVisible(false);
	btn->setCheckable(true);

	connect(btn->checkBox(), &QCheckBox::toggled, this, [=](bool b) {
		if(b)
			ch->enable();
		else
			ch->disable();
	});
	btn->checkBox()->setChecked(true);
}

void AdcInstrument::setupChannelsButtonHelper(MenuControlButton *channelsBtn)
{
	channelsBtn->setName("Channels");
	channelsBtn->setOpenMenuChecksThis(true);
	channelsBtn->setDoubleClickToOpenMenu(true);
	channelsBtn->checkBox()->setVisible(false);
	channelsBtn->setChecked(true);
	channelStack = new MapStackedWidget(this);
	tool->rightStack()->add(channelsMenuId, channelStack);
	connect(channelsBtn->button(), &QAbstractButton::toggled, this, [=](bool b) {
		if(b)
			tool->requestMenu(channelsMenuId);
	});
	rightMenuBtnGrp->addButton(channelsBtn->button());
}


void AdcInstrument::showMeasurements(bool b)
{
	if(b) {
		tool->requestMenu(measureMenuId);
		tool->requestMenu(statsMenuId);
	}
	tool->openTopContainerHelper(b);
	tool->openBottomContainerHelper(b);
}

void AdcInstrument::run(bool) {

}

void AdcInstrument::stop()
{

}

void AdcInstrument::start()
{

}

void AdcInstrument::restart()
{

}
