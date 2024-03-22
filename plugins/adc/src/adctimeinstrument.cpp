#include "adctimeinstrument.h"

using namespace scopy;
using namespace scopy::adc;
ADCTimeInstrument::ADCTimeInstrument(PlotProxy *proxy, QWidget *parent)
	: QWidget(parent)
	, proxy(proxy)
{

	setupToolLayout();
	proxy->setInstrument(this);
	init();
}

ADCTimeInstrument::~ADCTimeInstrument() { deinit(); }

void ADCTimeInstrument::setupToolLayout()
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
	runBtn = new RunBtn(this);
	singleBtn = new SingleShotBtn(this);

	channelsBtn = new MenuControlButton(this);

	tool->addWidgetToTopContainerMenuControlHelper(openLastMenuBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsBtn, TTA_LEFT);

	tool->addWidgetToTopContainerHelper(runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(singleBtn, TTA_RIGHT);

	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(printBtn, TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(channelsBtn, TTA_LEFT);

	rightMenuBtnGrp->addButton(settingsBtn);

	setupChannelsButtonHelper(channelsBtn);
	setupRunSingleButtonHelper();

	vcm->add(new QLabel("test"));

	connect(settingsBtn, &QPushButton::toggled, this, [=](bool b) {
		if(b)
			tool->requestMenu(settingsMenuId);
	});
}

void ADCTimeInstrument::setupRunSingleButtonHelper()
{
	/*connect(runBtn, &QPushButton::toggled, this, &ADCTimeInstrument::setRunning);
	connect(singleBtn, &QPushButton::toggled, plotAddon, &GRTimePlotAddon::setSingleShot);
	connect(singleBtn, &QPushButton::toggled, this, &ADCTimeInstrument::setRunning);
	connect(this, &ADCTimeInstrument::runningChanged, this, &ADCTimeInstrument::run);
	connect(this, &ADCTimeInstrument::runningChanged, runBtn, &QAbstractButton::setChecked);
	connect(plotAddon, &GRTimePlotAddon::requestStop, this, &AdcInstrument::stop, Qt::QueuedConnection);*/
}

void ADCTimeInstrument::setupChannelsButtonHelper(MenuControlButton *channelsBtn)
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

	connect(channelsBtn, &QPushButton::toggled, dynamic_cast<MenuHAnim *>(tool->leftContainer()),
		&MenuHAnim::toggleMenu);
	vcm = new VerticalChannelManager(this);
	tool->leftStack()->add(verticalChannelManagerId, vcm);
}

void ADCTimeInstrument::init() { proxy->init(); }

void ADCTimeInstrument::deinit() { proxy->deinit(); }

void ADCTimeInstrument::restart()
{
	if(m_running) {
		run(false);
		run(true);
	}
}

bool ADCTimeInstrument::running() const { return m_running; }

void ADCTimeInstrument::setRunning(bool newRunning)
{
	if(m_running == newRunning)
		return;
	m_running = newRunning;
	Q_EMIT runningChanged(newRunning);
}

ToolTemplate *ADCTimeInstrument::getToolTemplate() { return tool; }

MapStackedWidget *ADCTimeInstrument::getRightStack() { return rightStack; }

void ADCTimeInstrument::start() { run(true); }

void ADCTimeInstrument::stop() { run(false); }

void ADCTimeInstrument::run(bool b)
{

	if(!b) {
		runBtn->setChecked(false);
		singleBtn->setChecked(false);
	}

	qInfo() << b;
	QElapsedTimer tim;
	tim.start();

	if(b) {
		proxy->onStart();
	} else {
		proxy->onStop();
	}
}
