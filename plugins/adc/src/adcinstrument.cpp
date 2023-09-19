#include "adcinstrument.h"

#include <gui/widgets/toolbuttons.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/verticalchannelmanager.h>
#include <gui/widgets/measurementpanel.h>

using namespace scopy;
using namespace scopy::grutil;

AdcInstrument::AdcInstrument(PlotProxy* proxy, QWidget *parent) : QWidget(parent), proxy(proxy)
{
	static int uuid = 0;

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
	tool->bottomCentral()->setVisible(true);
	lay->addWidget(tool);

	openLastMenuBtn = new OpenLastMenuBtn(dynamic_cast<MenuHAnim*>(tool->rightContainer()),true,this);
	QButtonGroup* rightMenuBtnGrp = dynamic_cast<OpenLastMenuBtn*>(openLastMenuBtn)->getButtonGroup();
	tool->setLeftContainerWidth(210);
	tool->setRightContainerWidth(300);
	tool->setTopContainerHeight(90);
	tool->setBottomContainerHeight(90);

	GearBtn *settingsMenu = new GearBtn(this);
	InfoBtn *infoBtn = new InfoBtn(this);
	PrintBtn *printBtn = new PrintBtn(this);
	runBtn = new RunBtn(this);
	singleBtn = new SingleShotBtn(this);

	MenuControlButton *channelsBtn = new MenuControlButton(this);
	channelsBtn->setName("Channels");
	channelsBtn->checkBox()->setVisible(false);
	channelsBtn->button()->setVisible(false);
	channelsBtn->setChecked(true);

	MenuControlButton *cursor = new MenuControlButton(this);
	cursor->setName("Cursors");
	cursor->checkBox()->setVisible(false);
	cursor->setCheckBoxStyle(MenuControlButton::CS_SQUARE);
	MenuControlButton *measure = new MenuControlButton(this);

	// create measurement panel here ? or measurement addon ?
	measure->setName("Measure");
	measure->checkBox()->setVisible(false);
	measure->button()->setVisible(false);
	MeasurementsPanel* measure_panel = new MeasurementsPanel(this);
	tool->topStack()->add("measure", measure_panel);

	StatsPanel* stats_panel = new StatsPanel(this);
	tool->bottomStack()->add("stats", stats_panel);

	tool->addWidgetToTopContainerMenuControlHelper(openLastMenuBtn,TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsMenu,TTA_LEFT);

	tool->addWidgetToTopContainerHelper(runBtn,TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(singleBtn,TTA_RIGHT);

	tool->addWidgetToTopContainerHelper(infoBtn,TTA_LEFT);
	tool->addWidgetToTopContainerHelper(printBtn,TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(channelsBtn, TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(cursor, TTA_RIGHT);
	tool->addWidgetToBottomContainerHelper(measure, TTA_RIGHT);

	connect(channelsBtn, &QPushButton::toggled, dynamic_cast<MenuHAnim*>(tool->leftContainer()), &MenuHAnim::toggleMenu);

	GRTimePlotAddon* plotAddon = dynamic_cast<GRTimePlotAddon*>(proxy->getPlotAddon());
	tool->addWidgetToCentralContainerHelper(plotAddon->getWidget());

	GRTimePlotAddonSettings* plotAddonSettings = dynamic_cast<GRTimePlotAddonSettings*>(proxy->getPlotSettings());
	rightMenuBtnGrp->addButton(settingsMenu);
	QString settingsMenuId = plotAddonSettings->getName() + QString(uuid++);
	tool->rightStack()->add(settingsMenuId, plotAddonSettings->getWidget());
	connect(settingsMenu, &QPushButton::toggled, this, [=](bool b) {if(b) tool->requestMenu(settingsMenuId);});

	VerticalChannelManager *vcm = new VerticalChannelManager(this);
	tool->leftStack()->add("vcm",vcm);

	QButtonGroup *channelGroup = new QButtonGroup(this);
	for(auto d: proxy->getDeviceAddons()) {
		GRDeviceAddon *dev = dynamic_cast<GRDeviceAddon*>(d);
		if(!dev)
			return;
		CollapsableMenuControlButton *devBtn = new CollapsableMenuControlButton(this);
		QString id = dev->getName() + QString::number(uuid++);
		devBtn->getControlBtn()->setName(dev->getName());
		tool->rightStack()->add(id, dev->getWidget());
		connect(devBtn->getControlBtn()->button(), &QPushButton::toggled, this, [=](bool b) { if(b) tool->requestMenu(id);});
		rightMenuBtnGrp->addButton(devBtn->getControlBtn()->button());

		vcm->add(devBtn);
		for(GRTimeChannelAddon* ch : dev->getRegisteredChannels()) {
			MenuControlButton *btn = new MenuControlButton(devBtn);
			devBtn->add(btn);
			channelGroup->addButton(btn);

			btn->setName(ch->getName());
			btn->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
			btn->setColor(ch->pen().color());
			btn->setCheckable(true);

			QString id = ch->getName() + QString::number(uuid++);
			tool->rightStack()->add(id, ch->getWidget());
			connect(btn->checkBox(), &QCheckBox::toggled, this, [=](bool b) { if(b) ch->enable(); else ch->disable();} );
			connect(btn->button(), &QPushButton::toggled, this, [=](bool b) { if(b) tool->requestMenu(id);});
			connect(btn, &QAbstractButton::toggled, this, [=](){plotAddon->plot()->selectChannel(ch->plotCh());});

			connect(ch,&GRTimeChannelAddon::enableMeasurement, measure_panel, &MeasurementsPanel::addMeasurement);
			connect(ch,&GRTimeChannelAddon::disableMeasurement, measure_panel, &MeasurementsPanel::removeMeasurement);
			connect(ch,&GRTimeChannelAddon::enableStat, stats_panel, &StatsPanel::addStat);
			connect(ch,&GRTimeChannelAddon::disableStat, stats_panel, &StatsPanel::removeStat);

			rightMenuBtnGrp->addButton(btn->button());
			btn->checkBox()->setChecked(true);
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
	connect(cursor, &MenuControlButton::toggled, plotAddon,  &GRTimePlotAddon::showCursors);
	connect(measure, &MenuControlButton::toggled, this, &AdcInstrument::showMeasurements);

	tool->requestMenu("voltage02");
	channelGroup->buttons()[0]->setChecked(true);

	init();
}

AdcInstrument::~AdcInstrument()
{
	deinit();
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
		tool->requestMenu("measure");
		tool->requestMenu("stats");
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





