#include "adcinstrument.h"

using namespace scopy;
using namespace scopy::adc;
ADCInstrument::ADCInstrument(PlotProxy *proxy, QWidget *parent)
	: QWidget(parent)
	, proxy(proxy)
	, m_running(false)
{

	setupToolLayout();
	proxy->setInstrument(this);
	init();
}

ADCInstrument::~ADCInstrument() {
	stop();
	deinit();
}

void ADCInstrument::setupToolLayout()
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
	plotGroup = new QButtonGroup(this);

	tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);

	GearBtn *settingsBtn = new GearBtn(this);
	InfoBtn *infoBtn = new InfoBtn(this);
	// PrintBtn *printBtn = new PrintBtn(this);
	PrintPlotManager *printplotManager = new PrintPlotManager(this);
	runBtn = new RunBtn(this);
	singleBtn = new SingleShotBtn(this);

	channelsBtn = new MenuControlButton(this);

	timeBtn = new MenuControlButton(this);
	setupTimeButtonHelper(timeBtn);

	tool->addWidgetToTopContainerMenuControlHelper(openLastMenuBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsBtn, TTA_LEFT);

	tool->addWidgetToTopContainerHelper(runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(singleBtn, TTA_RIGHT);

	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(printplotManager->getPrintBtn(), TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(channelsBtn, TTA_LEFT);
	tool->addWidgetToBottomContainerHelper(timeBtn, TTA_LEFT);
	plotGroup->addButton(timeBtn);
	plotGroup->setExclusive(true);
	timeBtn->setChecked(true);

	rightMenuBtnGrp->addButton(settingsBtn);

	setupChannelsButtonHelper(channelsBtn);
	setupRunSingleButtonHelper();


	channelGroup = new QButtonGroup(this);

	m_vcm->add(new QLabel("test"));

	connect(settingsBtn, &QPushButton::toggled, this, [=](bool b) {
		if(b)
			tool->requestMenu(settingsMenuId);
	});
}


void ADCInstrument::setupRunSingleButtonHelper()
{
	connect(runBtn, &QPushButton::toggled, this, &ADCInstrument::setRunning);
	connect(singleBtn, &QPushButton::toggled, this, &ADCInstrument::setSingleShot);
	connect(singleBtn, &QPushButton::toggled, this, &ADCInstrument::setRunning);
	connect(this, &ADCInstrument::runningChanged, this, &ADCInstrument::run);
	connect(this, &ADCInstrument::runningChanged, runBtn, &QAbstractButton::setChecked);

	// connect(this, &ADCInstrument::requestStop, this, &ADCInstrument::stop, Qt::QueuedConnection);
}

void ADCInstrument::setupChannelsButtonHelper(MenuControlButton *channelsBtn)
{
	channelsBtn->setName("Channels");
	channelsBtn->setOpenMenuChecksThis(true);
	channelsBtn->setDoubleClickToOpenMenu(true);
	channelsBtn->checkBox()->setVisible(false);
	channelsBtn->setChecked(true);
	rightStack = new MapStackedWidget(this);
	tool->rightStack()->add(channelsMenuId, rightStack);
	connect(channelsBtn->button(), &QAbstractButton::toggled, this, [=](bool b) {
		if(b)
			tool->requestMenu(channelsMenuId);
	});
	rightMenuBtnGrp->addButton(channelsBtn->button());

	connect(printplotManager->getPrintBtn(), &QPushButton::clicked, this, [=, this]() {
		QList<PlotWidget *> plotList;
		plotList.push_back(plotAddon->plot());
		printplotManager->printPlots(plotList, "ADC");
	});

	connect(channelsBtn, &QPushButton::toggled, dynamic_cast<MenuHAnim *>(tool->leftContainer()),
		&MenuHAnim::toggleMenu);
	m_vcm = new VerticalChannelManager(this);
	tool->leftStack()->add(verticalChannelManagerId, m_vcm);
}

void ADCInstrument::setupTimeButtonHelper(MenuControlButton *time)
{
	time->setName("Time");
	time->button()->setVisible(false);
	time->setOpenMenuChecksThis(true);
	time->setDoubleClickToOpenMenu(true);
	time->checkBox()->setVisible(false);
	time->setCheckBoxStyle(MenuControlButton::CS_SQUARE);
}

void ADCInstrument::addDevice(CollapsableMenuControlButton *b, ToolComponent *dev)
{
	auto devBtn = b;
	m_vcm->add(b);
	QWidget *dev_widget = dynamic_cast<QWidget*>(dev);
	Q_ASSERT(dev_widget);

	channelGroup->addButton(b->getControlBtn());
	QString id = dev->name() + QString::number(uuid++);
	rightStack->add(id, dev_widget);

	connect(b->getControlBtn(), &QPushButton::clicked/* Or ::toggled*/ , this, [=](bool b) {
		if(b) {
			tool->requestMenu(channelsMenuId);
			rightStack->show(id);
		}
	});
}

/// ADD CHANNEL HERE

void ADCInstrument::addChannel(MenuControlButton *btn, ToolComponent *ch, CompositeWidget *c)
{
	c->add(btn);
	channelGroup->addButton(btn);

	QString id = ch->name() + QString::number(uuid++);
	QWidget *ch_widget = dynamic_cast<QWidget*>(ch);
	Q_ASSERT(ch_widget);

	rightStack->add(id, ch_widget);

	connect(btn, &QAbstractButton::clicked, this, [=](bool b) {
		if(b) {
			if(!channelsBtn->button()->isChecked()) {
				// Workaround because QButtonGroup and setChecked do not interact programatically
				channelsBtn->button()->animateClick(1);
			}
			tool->requestMenu(channelsMenuId);
			rightStack->show(id);
		}
	});

	/*setupChannelSnapshot(ch);
	setupChannelMeasurement(ch);
	setupChannelDelete(ch);*/
}
// #endif
void ADCInstrument::init() { proxy->init(); }

void ADCInstrument::deinit() { proxy->deinit(); }

MenuControlButton *ADCInstrument::getTimeBtn() const
{
	return timeBtn;
}

VerticalChannelManager *ADCInstrument::vcm() const
{
	return m_vcm;
}

void ADCInstrument::restart()
{
	if(m_running) {
		run(false);
		run(true);
	}
}

bool ADCInstrument::running() const { return m_running; }

void ADCInstrument::setRunning(bool newRunning)
{
	if(m_running == newRunning)
		return;
	m_running = newRunning;
	Q_EMIT runningChanged(newRunning);
}

ToolTemplate *ADCInstrument::getToolTemplate() { return tool; }

MapStackedWidget *ADCInstrument::getRightStack() { return rightStack; }

void ADCInstrument::start() { run(true); }

void ADCInstrument::stop() { run(false); }

void ADCInstrument::run(bool b)
{
	QSignalBlocker rb(runBtn);
	QSignalBlocker sb(singleBtn);

	runBtn->setChecked(b);
	singleBtn->setChecked(b);

	qInfo() << b;
	if(b) {
		proxy->onStart();
	} else {
		proxy->onStop();
	}
}
