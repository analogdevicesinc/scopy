#include "harmoniccalibration.h"

#include <stylehelper.h>

using namespace scopy;
using namespace scopy::grutil;

HarmonicCalibration::HarmonicCalibration(PlotProxy *proxy, QWidget *parent)
	: QWidget(parent)
	, proxy(proxy)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
    tool->setLeftContainerWidth(210);
	tool->setRightContainerWidth(300);
	tool->setTopContainerHeight(100);
	tool->setBottomContainerHeight(90);

    openLastMenuButton = new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(tool->rightContainer()), true, this);
	rightMenuButtonGroup = dynamic_cast<OpenLastMenuBtn *>(openLastMenuButton)->getButtonGroup();
    
    tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);

    settingsButton = new GearBtn(this);
    infoButton = new InfoBtn(this);
    runButton = new RunBtn(this);

    channelsButton = new MenuControlButton(this);
	setupChannelsButtonHelper(channelsButton);

    plotAddon = dynamic_cast<GRTimePlotAddon *>(proxy->getPlotAddon());
	tool->addWidgetToCentralContainerHelper(plotAddon->getWidget());

    plotAddonSettings = dynamic_cast<GRTimePlotAddonSettings *>(proxy->getPlotSettings());
	rightMenuButtonGroup->addButton(settingsButton);

    QString settingsMenuId = plotAddonSettings->getName() + QString(uuid++);
	tool->rightStack()->add(settingsMenuId, plotAddonSettings->getWidget());
	connect(settingsButton, &QPushButton::toggled, this, [=, this](bool b) {
		if(b)
			tool->requestMenu(settingsMenuId);
	});

    MenuControlButton *measure = new MenuControlButton(this);
	setupMeasureButtonHelper(measure);
	measurePanel = new MeasurementsPanel(this);
	tool->topStack()->add(measureMenuId, measurePanel);
    tool->openTopContainerHelper(false);

    statsPanel = new StatsPanel(this);
	tool->bottomStack()->add(statsMenuId, statsPanel);

    measureSettings = new MeasurementSettings(this);
	HoverWidget *measurePanelManagerHover = new HoverWidget(nullptr, measure, tool);
	measurePanelManagerHover->setContent(measureSettings);
	measurePanelManagerHover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	measurePanelManagerHover->setContentPos(HoverPosition::HP_TOPLEFT);
	connect(measure->button(), &QPushButton::toggled, this, [=, this](bool b) {
		measurePanelManagerHover->setVisible(b);
		measurePanelManagerHover->raise();
	});
    connect(measureSettings, &MeasurementSettings::enableMeasurementPanel, tool->topCentral(),
		&QWidget::setVisible);
	connect(measureSettings, &MeasurementSettings::enableStatsPanel, tool->bottomCentral(), &QWidget::setVisible);

	connect(measureSettings, &MeasurementSettings::sortMeasurements, measurePanel, &MeasurementsPanel::sort);
	connect(measureSettings, &MeasurementSettings::sortStats, statsPanel, &StatsPanel::sort);

    tool->addWidgetToTopContainerMenuControlHelper(openLastMenuButton, TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsButton, TTA_LEFT);

    tool->addWidgetToTopContainerHelper(infoButton, TTA_LEFT);
    tool->addWidgetToTopContainerHelper(runButton, TTA_RIGHT);

    tool->addWidgetToBottomContainerHelper(channelsButton, TTA_LEFT);
	tool->addWidgetToBottomContainerHelper(measure, TTA_RIGHT);

    connect(channelsButton, &QPushButton::toggled, dynamic_cast<MenuHAnim *>(tool->leftContainer()),
		&MenuHAnim::toggleMenu);

    // Left Channel Manager
    verticalChannelManager = new VerticalChannelManager(this);
	tool->leftStack()->add(verticalChannelManagerId, verticalChannelManager);

    channelGroup = new QButtonGroup(this);
	for(auto d : proxy->getDeviceAddons()) {
		GRDeviceAddon *dev = dynamic_cast<GRDeviceAddon *>(d);
		if(!dev)
			continue;
		CollapsableMenuControlButton *devButton = addDevice(dev, verticalChannelManager);
		verticalChannelManager->add(devButton);

		for(TimeChannelAddon *channelAddon : dev->getRegisteredChannels()) {
			auto menuControlButton = addChannel(channelAddon, devButton);
			devButton->add(menuControlButton);
		}
	}

	connect(runButton, &QPushButton::toggled, this, &HarmonicCalibration::setRunning);
	connect(this, &HarmonicCalibration::runningChanged, this, &HarmonicCalibration::run);
	connect(this, &HarmonicCalibration::runningChanged, runButton, &QAbstractButton::setChecked);
	connect(plotAddon, &GRTimePlotAddon::requestStop, this, &HarmonicCalibration::stop, Qt::QueuedConnection);
	connect(measure, &MenuControlButton::toggled, this, &HarmonicCalibration::showMeasurements);

	channelsButton->button()->setChecked(true);
	channelGroup->buttons()[1]->setChecked(true);

	init();
}

HarmonicCalibration::~HarmonicCalibration() {}

void HarmonicCalibration::setupChannelsButtonHelper(MenuControlButton *channelsButton)
{
	channelsButton->setName("Channels");
	channelsButton->setOpenMenuChecksThis(true);
	channelsButton->setDoubleClickToOpenMenu(true);
	channelsButton->checkBox()->setVisible(false);
	channelsButton->setChecked(true);
	channelStack = new MapStackedWidget(this);
	tool->rightStack()->add(channelsMenuId, channelStack);
	connect(channelsButton->button(), &QAbstractButton::toggled, this, [=, this](bool b) {
		if(b)
			tool->requestMenu(channelsMenuId);
	});
	rightMenuButtonGroup->addButton(channelsButton->button());
}

void HarmonicCalibration::setupMeasureButtonHelper(MenuControlButton *measureButton)
{
	measureButton->setName("Measure");
	measureButton->setOpenMenuChecksThis(true);
	measureButton->setDoubleClickToOpenMenu(true);
	measureButton->checkBox()->setVisible(false);
}

void HarmonicCalibration::setupChannelMenuControlButtonHelper(MenuControlButton *menuControlButton, ChannelAddon *channelAddon)
{
	menuControlButton->setName(channelAddon->getName());
	menuControlButton->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	menuControlButton->setOpenMenuChecksThis(true);
	menuControlButton->setDoubleClickToOpenMenu(true);
	menuControlButton->setColor(channelAddon->pen().color());
	menuControlButton->button()->setVisible(false);
	menuControlButton->setCheckable(true);

	connect(menuControlButton->checkBox(), &QCheckBox::toggled, this, [=, this](bool b) {
		if(b)
			channelAddon->enable();
		else
			channelAddon->disable();
	});
	menuControlButton->checkBox()->setChecked(true);
}

void HarmonicCalibration::setupChannelSnapshot(ChannelAddon *channelAddon)
{
	auto snapshotChannel = dynamic_cast<SnapshotProvider *>(channelAddon);
	if(!snapshotChannel)
		return;
	connect(channelAddon, SIGNAL(addNewSnapshot(SnapshotProvider::SnapshotRecipe)), this,
		SLOT(createSnapshotChannel(SnapshotProvider::SnapshotRecipe)));
}

void HarmonicCalibration::setupChannelMeasurement(ChannelAddon *channelAddon)
{
	auto chMeasureableChannel = dynamic_cast<MeasurementProvider *>(channelAddon);
	if(!chMeasureableChannel)
		return;
	auto chMeasureManager = chMeasureableChannel->getMeasureManager();
	if(!chMeasureManager)
		return;
	if(measureSettings) {
		connect(chMeasureManager, &MeasureManagerInterface::enableMeasurement, measurePanel,
			&MeasurementsPanel::addMeasurement);
		connect(chMeasureManager, &MeasureManagerInterface::disableMeasurement, measurePanel,
			&MeasurementsPanel::removeMeasurement);
		connect(measureSettings, &MeasurementSettings::toggleAllMeasurements, chMeasureManager,
			&MeasureManagerInterface::toggleAllMeasurement);
		connect(measureSettings, &MeasurementSettings::toggleAllStats, chMeasureManager,
			&MeasureManagerInterface::toggleAllStats);
		connect(chMeasureManager, &MeasureManagerInterface::enableStat, statsPanel, &StatsPanel::addStat);
		connect(chMeasureManager, &MeasureManagerInterface::disableStat, statsPanel, &StatsPanel::removeStat);
	}
}

void HarmonicCalibration::setupChannelDelete(ChannelAddon *channelAddon)
{
	connect(channelAddon, SIGNAL(requestDeleteChannel(ChannelAddon *)), this, SLOT(deleteChannel(ChannelAddon *)));
}

void HarmonicCalibration::deleteChannel(ChannelAddon *ch)
{

	MenuControlButton *last = nullptr;
	for(auto c : proxy->getChannelAddons()) {
		auto ca = dynamic_cast<ChannelAddon *>(c);
		if(ca == ch && last) {
			last->animateClick(1);
		}

		last = dynamic_cast<MenuControlButton *>(ca->getMenuControlWidget());
	}
	proxy->removeChannelAddon(ch);

	ch->onStop();
	ch->disable();
	plotAddon->onChannelRemoved(ch);
	plotAddonSettings->onChannelRemoved(ch);
	ch->onDeinit();
	delete ch->getMenuControlWidget();
	delete ch;
}

MenuControlButton *HarmonicCalibration::addChannel(ChannelAddon *channelAddon, QWidget *parent)
{
	MenuControlButton *menuControlButton = new MenuControlButton(parent);
	channelAddon->setMenuControlWidget(menuControlButton);
	channelGroup->addButton(menuControlButton);

	QString id = channelAddon->getName() + QString::number(uuid++);
	setupChannelMenuControlButtonHelper(menuControlButton, channelAddon);

	channelStack->add(id, channelAddon->getWidget());

	connect(menuControlButton, &QAbstractButton::clicked, this, [=, this](bool b) {
		if(b) {
			if(!channelsButton->button()->isChecked()) {
				// Workaround because QButtonGroup and setChecked do not interact programatically
				channelsButton->button()->animateClick(1);
			}

			plotAddon->plot()->selectChannel(channelAddon->plotCh());
			channelStack->show(id);
		}
	});

	setupChannelSnapshot(channelAddon);
	setupChannelMeasurement(channelAddon);
	setupChannelDelete(channelAddon);
	plotAddon->onChannelAdded(channelAddon);
	plotAddonSettings->onChannelAdded(channelAddon);
	return menuControlButton;
}

void HarmonicCalibration::setupDeviceMenuControlButtonHelper(MenuControlButton *deviceMenuControlButton, GRDeviceAddon *deviceAddon)
{
	deviceMenuControlButton->setName(deviceAddon->getName());
	deviceMenuControlButton->setCheckable(true);
	deviceMenuControlButton->button()->setVisible(false);
	deviceMenuControlButton->setOpenMenuChecksThis(true);
	deviceMenuControlButton->setDoubleClickToOpenMenu(true);
}

CollapsableMenuControlButton *HarmonicCalibration::addDevice(GRDeviceAddon *dev, QWidget *parent)
{
	auto devButton = new CollapsableMenuControlButton(parent);
	setupDeviceMenuControlButtonHelper(devButton->getControlBtn(), dev);
	channelGroup->addButton(devButton->getControlBtn());
	QString id = dev->getName() + QString::number(uuid++);
	channelStack->add(id, dev->getWidget());
	connect(devButton->getControlBtn(), &QPushButton::toggled, this, [=, this](bool b) {
		if(b) {
			tool->requestMenu(channelsMenuId);
			channelStack->show(id);
		}
	});
	return devButton;
}

void HarmonicCalibration::init()
{
	auto addons = proxy->getAddons();
	proxy->init();
	//initCursors();
	for(auto addon : addons) {
		addon->onInit();
	}
}

void HarmonicCalibration::deinit()
{
	auto addons = proxy->getAddons();

	for(auto addon : addons) {
		addon->onDeinit();
	}
}

void HarmonicCalibration::restart()
{
	if(m_running) {
		run(false);
		run(true);
	}
}

void HarmonicCalibration::showMeasurements(bool b)
{
	if(b) {
		tool->requestMenu(measureMenuId);
		tool->requestMenu(statsMenuId);
	}
	tool->openTopContainerHelper(b);
	tool->openBottomContainerHelper(b);
}

void HarmonicCalibration::createSnapshotChannel(SnapshotProvider::SnapshotRecipe rec)
{
	//	proxy->getChannelAddons().append(new ch)
	qInfo() << "Creating snapshot from recipe" << rec.name;

	ChannelIdProvider *chidp = proxy->getChannelIdProvider();
	int idx = chidp->next();
	ImportChannelAddon *ch = new ImportChannelAddon("REF-" + rec.name + "-" + QString::number(idx), plotAddon,
							chidp->pen(idx), this);
	proxy->addChannelAddon(ch);
	ch->setData(rec.x, rec.y);
	auto btn = addChannel(ch, verticalChannelManager);
	verticalChannelManager->add(btn);
	ch->onInit();
	btn->animateClick(1);
}

bool HarmonicCalibration::running() const { return m_running; }

void HarmonicCalibration::setRunning(bool newRunning)
{
	if(m_running == newRunning)
		return;
	m_running = newRunning;
	Q_EMIT runningChanged(newRunning);
}

void HarmonicCalibration::start() { run(true); }

void HarmonicCalibration::stop() { run(false); }

void HarmonicCalibration::startAddons()
{
	auto addons = proxy->getAddons();

	for(auto addon : addons) {
		addon->onStart();
	}
}
void HarmonicCalibration::stopAddons()
{
	auto addons = proxy->getAddons();

	for(auto addon : addons) {
		addon->onStop();
	}
}

void HarmonicCalibration::run(bool b)
{
	qInfo() << b;
	QElapsedTimer tim;
	tim.start();

	if(!b) {
		runButton->setChecked(false);
	}

	if(b) {
		startAddons();
	} else {
		stopAddons();
	}
}