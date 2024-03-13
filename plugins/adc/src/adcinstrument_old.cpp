#include "adcinstrument_old.h"

#include "gui/widgets/measurementsettings.h"

#include <gui/widgets/hoverwidget.h>
#include <gui/widgets/measurementpanel.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/toolbuttons.h>
#include <gui/widgets/verticalchannelmanager.h>

using namespace scopy;
using namespace scopy::grutil;

AdcInstrument_Old::AdcInstrument_Old(PlotProxy *proxy, QWidget *parent)
	: QWidget(parent)
	, proxy(proxy)
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

	QPushButton *addBtn = new QPushButton("+",this);
	QPushButton *removeBtn = new QPushButton("X",this);

	runBtn = new RunBtn(this);
	singleBtn = new SingleShotBtn(this);

	channelsBtn = new MenuControlButton(this);
	setupChannelsButtonHelper(channelsBtn);

	//	MenuControlButton *timeBtn = new MenuControlButton(this);
	//	setupTimeButtonHelper(timeBtn);

	//	MenuControlButton *xyBtn = new MenuControlButton(this);
	//	setupXyButtonHelper(xyBtn);

	//	MenuControlButton *fftBtn = new MenuControlButton(this);
	//	setupFFTButtonHelper(fftBtn);

	plotAddon = dynamic_cast<GRTimePlotAddon *>(proxy->getPlotAddon());
	tool->addWidgetToCentralContainerHelper(plotAddon->getWidget());

	plotAddonSettings = dynamic_cast<GRTimePlotAddonSettings *>(proxy->getPlotSettings());
	rightMenuBtnGrp->addButton(settingsBtn);

	QString settingsMenuId = plotAddonSettings->getName() + QString(uuid++);
	tool->rightStack()->add(settingsMenuId, plotAddonSettings->getWidget());
	connect(settingsBtn, &QPushButton::toggled, this, [=](bool b) {
		if(b)
			tool->requestMenu(settingsMenuId);
	});

	MenuControlButton *cursor = new MenuControlButton(this);
	setupCursorButtonHelper(cursor);

	cursorController = new CursorController(plotAddon->plot(), this);
	fftcursorController = new CursorController(plotAddon->fftplot(), this);
	fftcursorController->getCursorSettings()->hide();

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
	tool->addWidgetToTopContainerHelper(addBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(removeBtn, TTA_LEFT);

	removeBtn->setEnabled(false);

	connect(addBtn,&QPushButton::clicked,this,[=](){Q_EMIT newPlot(0);});
	connect(removeBtn,&QPushButton::clicked,this,[=](){Q_EMIT removePlot(this);});


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

	channelGroup = new QButtonGroup(this);
	for(auto d : proxy->getDeviceAddons()) {
		GRDeviceAddon *dev = dynamic_cast<GRDeviceAddon *>(d);
		if(!dev)
			continue;
		CollapsableMenuControlButton *devBtn = addDevice(dev, vcm);
		vcm->add(devBtn);

		for(TimeChannelAddon *ch : dev->getRegisteredChannels()) {
			auto btn = addChannel(ch, devBtn);
			devBtn->add(btn);
		}
	}

	for(ToolAddon *c : proxy->getChannelAddons()) {
		bool needsBuild = true;
		ChannelAddon *ch = dynamic_cast<ChannelAddon *>(c);
		if(!ch)
			continue;

		// this is hacky - Needs categories as part of the channelAddon (?)
		// review this on refactor
		GRTimeChannelAddon *gtch = dynamic_cast<GRTimeChannelAddon *>(ch);
		if(gtch) {
			for(auto d : proxy->getDeviceAddons()) {
				GRDeviceAddon *dev = dynamic_cast<GRDeviceAddon *>(d);
				if(!dev)
					continue;
				if(dev->getRegisteredChannels().contains(gtch)) {
					needsBuild = false;
					continue;
				}
			}
		}

		if(!needsBuild)
			continue;
		auto btn = addChannel(ch, vcm);
		vcm->add(btn);
	}

	connect(runBtn, &QPushButton::toggled, this, &AdcInstrument_Old::setRunning);
	connect(singleBtn, &QPushButton::toggled, plotAddon, &GRTimePlotAddon::setSingleShot);
	connect(singleBtn, &QPushButton::toggled, this, &AdcInstrument_Old::setRunning);
	connect(this, &AdcInstrument_Old::runningChanged, this, &AdcInstrument_Old::run);
	connect(this, &AdcInstrument_Old::runningChanged, runBtn, &QAbstractButton::setChecked);

	connect(plotAddon, &GRTimePlotAddon::requestStop, this, &AdcInstrument_Old::stop, Qt::QueuedConnection);
	connect(cursor->button(), &QAbstractButton::toggled, hoverSettings, &HoverWidget::setVisible);
	connect(cursor, &QAbstractButton::toggled, cursorController, &CursorController::setVisible);
	connect(cursor, &QAbstractButton::toggled, this, [=](bool b) {
		//		fftcursorController->setVisible(b);

		//		plotAddon->fftplot()->leftHandlesArea()->setVisible(true);
		//		plotAddon->fftplot()->bottomHandlesArea()->setVisible(b);
		//		plotAddon->fftplot()->bottomHandlesArea()->setLeftPadding(60);
	});
	connect(measure, &MenuControlButton::toggled, this, &AdcInstrument_Old::showMeasurements);

	channelStack->show("voltage02");
	channelsBtn->button()->setChecked(true);
	channelGroup->buttons()[1]->setChecked(true);

	init();
}

void AdcInstrument_Old::initCursors()
{
	cursorController->getPlotCursors()->getH1Cursor()->setPosition(0);
	cursorController->getPlotCursors()->getH2Cursor()->setPosition(0);
	cursorController->getPlotCursors()->getV1Cursor()->setPosition(0);
	cursorController->getPlotCursors()->getV2Cursor()->setPosition(0);
}

AdcInstrument_Old::~AdcInstrument_Old() { deinit(); }

void AdcInstrument_Old::setupChannelSnapshot(ChannelAddon *ch)
{
	auto snapshotChannel = dynamic_cast<SnapshotProvider *>(ch);
	if(!snapshotChannel)
		return;
	connect(ch, SIGNAL(addNewSnapshot(SnapshotProvider::SnapshotRecipe)), this,
		SLOT(createSnapshotChannel(SnapshotProvider::SnapshotRecipe)));
}

void AdcInstrument_Old::setupChannelDelete(ChannelAddon *ch)
{
	connect(ch, SIGNAL(requestDeleteChannel(ChannelAddon *)), this, SLOT(deleteChannel(ChannelAddon *)));
}

void AdcInstrument_Old::deleteChannel(ChannelAddon *ch)
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

void AdcInstrument_Old::setupChannelMeasurement(ChannelAddon *ch)
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

MenuControlButton *AdcInstrument_Old::addChannel(ChannelAddon *ch, QWidget *parent)
{
	MenuControlButton *btn = new MenuControlButton(parent);
	ch->setMenuControlWidget(btn);
	channelGroup->addButton(btn);

	QString id = ch->getName() + QString::number(uuid++);
	setupChannelMenuControlButtonHelper(btn, ch);

	channelStack->add(id, ch->getWidget());

	connect(btn, &QAbstractButton::clicked, this, [=](bool b) {
		if(b) {
			if(!channelsBtn->button()->isChecked()) {
				// Workaround because QButtonGroup and setChecked do not interact programatically
				channelsBtn->button()->animateClick(1);
			}

			plotAddon->plot()->selectChannel(ch->plotCh());
			channelStack->show(id);
		}
	});

	setupChannelSnapshot(ch);
	setupChannelMeasurement(ch);
	setupChannelDelete(ch);
	plotAddon->onChannelAdded(ch);
	plotAddonSettings->onChannelAdded(ch);
	return btn;
}

CollapsableMenuControlButton *AdcInstrument_Old::addDevice(GRDeviceAddon *dev, QWidget *parent)
{
	auto devBtn = new CollapsableMenuControlButton(parent);
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
	return devBtn;
}

void AdcInstrument_Old::setupTimeButtonHelper(MenuControlButton *time)
{
	time->setName("Time");
	time->setOpenMenuChecksThis(true);
	time->setDoubleClickToOpenMenu(true);
	time->checkBox()->setVisible(false);
	time->setCheckBoxStyle(MenuControlButton::CS_SQUARE);
}

void AdcInstrument_Old::setupXyButtonHelper(MenuControlButton *xy)
{
	xy->setName("X-Y");
	xy->setOpenMenuChecksThis(true);
	xy->setDoubleClickToOpenMenu(true);
	xy->checkBox()->setVisible(false);
	xy->setCheckBoxStyle(MenuControlButton::CS_SQUARE);
}

void AdcInstrument_Old::setupFFTButtonHelper(MenuControlButton *fft)
{
	fft->setName("FFT");
	fft->setOpenMenuChecksThis(true);
	fft->setDoubleClickToOpenMenu(true);
	fft->checkBox()->setVisible(false);
	fft->setCheckBoxStyle(MenuControlButton::CS_SQUARE);
}

void AdcInstrument_Old::setupCursorButtonHelper(MenuControlButton *cursor)
{
	cursor->setName("Cursors");
	cursor->setOpenMenuChecksThis(true);
	cursor->setDoubleClickToOpenMenu(true);
	cursor->checkBox()->setVisible(false);
	cursor->setCheckBoxStyle(MenuControlButton::CS_SQUARE);
}

void AdcInstrument_Old::setupMeasureButtonHelper(MenuControlButton *btn)
{
	btn->setName("Measure");
	btn->setOpenMenuChecksThis(true);
	btn->setDoubleClickToOpenMenu(true);
	btn->checkBox()->setVisible(false);
}

void AdcInstrument_Old::setupDeviceMenuControlButtonHelper(MenuControlButton *devBtn, GRDeviceAddon *dev)
{
	devBtn->setName(dev->getName());
	devBtn->setCheckable(true);
	devBtn->button()->setVisible(false);
	devBtn->setOpenMenuChecksThis(true);
	devBtn->setDoubleClickToOpenMenu(true);
}

void AdcInstrument_Old::setupChannelMenuControlButtonHelper(MenuControlButton *btn, ChannelAddon *ch)
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

void AdcInstrument_Old::setupChannelsButtonHelper(MenuControlButton *channelsBtn)
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

void AdcInstrument_Old::init()
{
	auto addons = proxy->getAddons();
	proxy->init();
	initCursors();
	for(auto addon : addons) {
		addon->onInit();
	}
}

void AdcInstrument_Old::deinit()
{
	auto addons = proxy->getAddons();

	for(auto addon : addons) {
		addon->onDeinit();
	}
}

void AdcInstrument_Old::restart()
{
	if(m_running) {
		run(false);
		run(true);
	}
}

void AdcInstrument_Old::showMeasurements(bool b)
{
	if(b) {
		tool->requestMenu(measureMenuId);
		tool->requestMenu(statsMenuId);
	}
	tool->openTopContainerHelper(b);
	tool->openBottomContainerHelper(b);
}

void AdcInstrument_Old::createSnapshotChannel(SnapshotProvider::SnapshotRecipe rec)
{
	//	proxy->getChannelAddons().append(new ch)
	qInfo() << "Creating snapshot from recipe" << rec.name;

	ChannelIdProvider *chidp = proxy->getChannelIdProvider();
	int idx = chidp->next();
	ImportChannelAddon *ch = new ImportChannelAddon("REF-" + rec.name + "-" + QString::number(idx), plotAddon,
							chidp->pen(idx), this);
	proxy->addChannelAddon(ch);
	ch->setData(rec.x, rec.y);
	auto btn = addChannel(ch, vcm);
	vcm->add(btn);
	ch->onInit();
	btn->animateClick(1);
}

bool AdcInstrument_Old::running() const { return m_running; }

void AdcInstrument_Old::setRunning(bool newRunning)
{
	if(m_running == newRunning)
		return;
	m_running = newRunning;
	Q_EMIT runningChanged(newRunning);
}

void AdcInstrument_Old::start() { run(true); }

void AdcInstrument_Old::stop() { run(false); }

void AdcInstrument_Old::startAddons()
{
	auto addons = proxy->getAddons();

	for(auto addon : addons) {
		addon->onStart();
	}
}
void AdcInstrument_Old::stopAddons()
{
	auto addons = proxy->getAddons();

	for(auto addon : addons) {
		addon->onStop();
	}
}

void AdcInstrument_Old::run(bool b)
{
	qInfo() << b;
	QElapsedTimer tim;
	tim.start();

	if(!b) {
		runBtn->setChecked(false);
		singleBtn->setChecked(false);
	}

	if(b) {
		startAddons();
	} else {
		stopAddons();
	}
}
