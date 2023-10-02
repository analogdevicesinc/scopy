#include "adcinstrument.h"

#include <gui/widgets/toolbuttons.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/verticalchannelmanager.h>
#include <gui/widgets/measurementpanel.h>
#include <gui/widgets/hoverwidget.h>
#include "gui/widgets/measurementsettings.h"
#include <cursorcontroller.h>

using namespace scopy;
using namespace scopy::grutil;

AdcInstrument::AdcInstrument(PlotProxy* proxy, QWidget *parent) : QWidget(parent), proxy(proxy)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *lay = new QHBoxLayout(this);
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

	openLastMenuBtn = new OpenLastMenuBtn(dynamic_cast<MenuHAnim*>(tool->rightContainer()),true,this);
	rightMenuBtnGrp = dynamic_cast<OpenLastMenuBtn*>(openLastMenuBtn)->getButtonGroup();

	tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);

	GearBtn *settingsBtn = new GearBtn(this);
	InfoBtn *infoBtn = new InfoBtn(this);
	PrintBtn *printBtn = new PrintBtn(this);
	runBtn = new RunBtn(this);
	singleBtn = new SingleShotBtn(this);

	channelsBtn = new MenuControlButton(this);
	setupChannelsButtonHelper(channelsBtn);

	plotAddon = dynamic_cast<GRTimePlotAddon*>(proxy->getPlotAddon());
	tool->addWidgetToCentralContainerHelper(plotAddon->getWidget());

	plotAddonSettings = dynamic_cast<GRTimePlotAddonSettings*>(proxy->getPlotSettings());
	rightMenuBtnGrp->addButton(settingsBtn);

	QString settingsMenuId = plotAddonSettings->getName() + QString(uuid++);
	tool->rightStack()->add(settingsMenuId, plotAddonSettings->getWidget());
	connect(settingsBtn, &QPushButton::toggled, this, [=](bool b) {
		if(b)
			tool->requestMenu(settingsMenuId);
	});

	MenuControlButton *cursor = new MenuControlButton(this);
	setupCursorButtonHelper(cursor);

	CursorController *cursorController = new CursorController(plotAddon->plot(), this);
	HoverWidget *hoverSettings = new HoverWidget(cursorController->getCursorSettings(), cursor, tool);
	hoverSettings->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	hoverSettings->setContentPos(HoverPosition::HP_TOPLEFT);
	hoverSettings->setAnchorOffset(QPoint(0, -10));

	MenuControlButton *measure = new MenuControlButton(this);
	setupMeasureButtonHelper(measure);
	measure_panel = new MeasurementsPanel(this);
	tool->topStack()->add(measureMenuId, measure_panel);

	stats_panel = new StatsPanel(this);
	tool->bottomStack()->add(statsMenuId, stats_panel);

	MeasurementSettings* measureSettings = new MeasurementSettings(this);
	HoverWidget *measurePanelManagerHover = new HoverWidget(nullptr, measure, tool);
	measurePanelManagerHover->setContent(measureSettings);
	measurePanelManagerHover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	measurePanelManagerHover->setContentPos(HoverPosition::HP_TOPLEFT);
	connect(measure->button(), &QPushButton::toggled, this, [=](bool b) {
		measurePanelManagerHover->setVisible(b);
		measurePanelManagerHover->raise();
	});
	connect(measureSettings, &MeasurementSettings::enableMeasurementPanel, tool->topCentral(), &QWidget::setVisible);
	connect(measureSettings, &MeasurementSettings::enableStatsPanel, tool->bottomCentral(), &QWidget::setVisible);

	connect(measureSettings, &MeasurementSettings::sortMeasurements, measure_panel, &MeasurementsPanel::sort);
	connect(measureSettings, &MeasurementSettings::sortStats, stats_panel, &StatsPanel::sort);

	tool->addWidgetToTopContainerMenuControlHelper(openLastMenuBtn,TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsBtn,TTA_LEFT);

	tool->addWidgetToTopContainerHelper(runBtn,TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(singleBtn,TTA_RIGHT);

	tool->addWidgetToTopContainerHelper(infoBtn,TTA_LEFT);
	tool->addWidgetToTopContainerHelper(printBtn,TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(channelsBtn, TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(cursor, TTA_RIGHT);
	tool->addWidgetToBottomContainerHelper(measure, TTA_RIGHT);

	connect(channelsBtn, &QPushButton::toggled, dynamic_cast<MenuHAnim*>(tool->leftContainer()), &MenuHAnim::toggleMenu);

	VerticalChannelManager *vcm = new VerticalChannelManager(this);
	tool->leftStack()->add(verticalChannelManagerId, vcm);

	channelGroup = new QButtonGroup(this);
	for(auto d: proxy->getDeviceAddons()) {
		GRDeviceAddon *dev = dynamic_cast<GRDeviceAddon*>(d);
		if(!dev)
			return;
		CollapsableMenuControlButton *devBtn = new CollapsableMenuControlButton(this);
		setupDeviceMenuControlButtonHelper(devBtn->getControlBtn(), dev);
		channelGroup->addButton(devBtn->getControlBtn());
		QString id = dev->getName() + QString::number(uuid++);
		channelStack->add(id, dev->getWidget());
		connect(devBtn->getControlBtn(), &QPushButton::toggled, this, [=](bool b) {
			if(b) {
				tool->requestMenu(channelsMenuId);
				channelStack->show(id);
			}
		});
		vcm->add(devBtn);

		for(GRTimeChannelAddon* ch : dev->getRegisteredChannels()) {
			MenuControlButton *btn = new MenuControlButton(devBtn);
			devBtn->add(btn);
			channelGroup->addButton(btn);

			QString id = ch->getName() + QString::number(uuid++);
			setupChannelMenuControlButtonHelper(btn, ch);

			channelStack->add(id, ch->getWidget());

			connect(btn, &QAbstractButton::clicked, this, [=](bool b){
				if(b) {
					if(!channelsBtn->button()->isChecked()) {
						// Workaround because QButtonGroup and setChecked do not interact programatically
						channelsBtn->button()->animateClick(1);
					}

					plotAddon->plot()->selectChannel(ch->plotCh());
					channelStack->show(id);
				}
			});

			connect(ch,&GRTimeChannelAddon::enableMeasurement, measure_panel, &MeasurementsPanel::addMeasurement);
			connect(ch,&GRTimeChannelAddon::disableMeasurement, measure_panel, &MeasurementsPanel::removeMeasurement);
			connect(measureSettings, &MeasurementSettings::toggleAllMeasurements, ch, &GRTimeChannelAddon::toggleAllMeasurement);
			connect(measureSettings, &MeasurementSettings::toggleAllStats, ch, &GRTimeChannelAddon::toggleAllStats);
			connect(ch,&GRTimeChannelAddon::enableStat, stats_panel, &StatsPanel::addStat);
			connect(ch,&GRTimeChannelAddon::disableStat, stats_panel, &StatsPanel::removeStat);

			plotAddon->onChannelAdded(ch);
			plotAddonSettings->onChannelAdded(ch);

		}
	}

	connect(runBtn,&QPushButton::toggled, this, &AdcInstrument::setRunning);
	connect(singleBtn,&QPushButton::toggled, plotAddon, &GRTimePlotAddon::setSingleShot);
	connect(singleBtn,&QPushButton::toggled, this, &AdcInstrument::setRunning);
	connect(this, &AdcInstrument::runningChanged, this, &AdcInstrument::run);
	connect(this, &AdcInstrument::runningChanged, runBtn, &QAbstractButton::setChecked);

	connect(plotAddon, &GRTimePlotAddon::requestStop, this, &AdcInstrument::stop, Qt::QueuedConnection);
	connect(cursor->button(), &QAbstractButton::toggled, hoverSettings, &HoverWidget::setVisible);
	connect(cursor, &QAbstractButton::toggled, cursorController, &CursorController::setVisible);
	connect(measure, &MenuControlButton::toggled, this, &AdcInstrument::showMeasurements);

	channelStack->show("voltage02");
	channelsBtn->button()->setChecked(true);
	channelGroup->buttons()[1]->setChecked(true);

	init();
}

AdcInstrument::~AdcInstrument()
{
	deinit();
}

void AdcInstrument::setupCursorButtonHelper(MenuControlButton *cursor) {
	cursor->setName("Cursors");
	cursor->setOpenMenuChecksThis(true);
	cursor->setDoubleClickToOpenMenu(true);
	cursor->checkBox()->setVisible(false);
	cursor->setCheckBoxStyle(MenuControlButton::CS_SQUARE);
}

void AdcInstrument::setupMeasureButtonHelper(MenuControlButton *btn) {
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

void AdcInstrument::setupChannelMenuControlButtonHelper(MenuControlButton *btn, GRTimeChannelAddon *ch) {
	btn->setName(ch->getName());
	btn->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	btn->setOpenMenuChecksThis(true);
	btn->setDoubleClickToOpenMenu(true);
	btn->setColor(ch->pen().color());
	btn->button()->setVisible(false);
	btn->setCheckable(true);

	connect(btn->checkBox(), &QCheckBox::toggled, this, [=](bool b) {
		if(b) ch->enable(); else ch->disable();
	} );
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
	tool->rightStack()->add(channelsMenuId,channelStack);
	connect(channelsBtn->button(), &QAbstractButton::toggled, this, [=](bool b){
		if(b)
			tool->requestMenu(channelsMenuId);
	});
	rightMenuBtnGrp->addButton(channelsBtn->button());
}

void AdcInstrument::init() {
	auto addons = proxy->getAddons();
	proxy->init();
	for(auto addon : addons) {
		addon->onInit();
	}
}

void AdcInstrument::deinit() {
	auto addons = proxy->getAddons();

	for(auto addon : addons) {
			addon->onDeinit();
	}
}

void AdcInstrument::restart() {
	if(m_running) {
			run(false);
			run(true);
	}
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

bool AdcInstrument::running() const
{
	return m_running;
}

void AdcInstrument::setRunning(bool newRunning)
{
	if (m_running == newRunning)
			return;
	m_running = newRunning;
	Q_EMIT runningChanged(newRunning);
}

void AdcInstrument::start() {
	run(true);
}

void AdcInstrument::stop() {
	run(false);
}

void AdcInstrument::startAddons() {
	auto addons = proxy->getAddons();

	for(auto addon : addons) {
			addon->onStart();
	}
}
void AdcInstrument::stopAddons() {
	auto addons = proxy->getAddons();

	for(auto addon : addons) {
			addon->onStop();
	}
}

void AdcInstrument::run(bool b) {
	qInfo()<<b;
	QElapsedTimer tim;
	tim.start();

	if(!b) {
		runBtn->setChecked(false);
		singleBtn->setChecked(false);
	}

	if(b)  {
		startAddons();
	} else {
		stopAddons();
	}
}





