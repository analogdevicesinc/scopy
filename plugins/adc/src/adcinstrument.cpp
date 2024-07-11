#include "adcinstrument.h"
#include <pluginbase/resourcemanager.h>
#include <gui/smallOnOffSwitch.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_ADCINSTRUMENT, "ADCInstrument")

using namespace scopy;
using namespace scopy::adc;



ADCInstrument::ADCInstrument(ToolMenuEntry *tme, QWidget *parent)
	: QWidget(parent)
	, m_tme(tme)
{
	setupToolLayout();
}

ADCInstrument::~ADCInstrument()
{
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

	tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);

	GearBtn *settingsBtn = new GearBtn(this);
	InfoBtn *infoBtn = new InfoBtn(this);
	// PrintBtn *printBtn = new PrintBtn(this);
	PrintPlotManager *printplotManager = new PrintPlotManager(this);
	addBtn = new AddBtn(this);
	removeBtn = new RemoveBtn(this);

	m_sync = new QPushButton("Sync");
	m_sync->setCheckable(true);
	StyleHelper::BlueGrayButton(m_sync);

	m_complex = new QPushButton("Complex");
	m_complex->setCheckable(true);
	StyleHelper::BlueGrayButton(m_complex);

	m_runBtn = new RunBtn(this);
	m_singleBtn = new SingleShotBtn(this);

	channelsBtn = new MenuControlButton(this);

	tool->addWidgetToTopContainerMenuControlHelper(openLastMenuBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsBtn, TTA_LEFT);

	tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);

	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(printplotManager->getPrintBtn(), TTA_LEFT);

	tool->addWidgetToTopContainerHelper(addBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(removeBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(m_sync, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(m_complex, TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(channelsBtn, TTA_LEFT);

	rightMenuBtnGrp->addButton(settingsBtn);

	setupChannelsButtonHelper(channelsBtn);
	setupRunSingleButtonHelper();

	channelGroup = new QButtonGroup(this);

	connect(settingsBtn, &QPushButton::toggled, this, [=](bool b) {
		if(b)
			tool->requestMenu(settingsMenuId);
	});

	connect(m_runBtn, &QAbstractButton::toggled, this, [=](bool b) {
		m_runBtn->setEnabled(false);
		if(b) {
			Q_EMIT requestStart();
		} else {
			Q_EMIT requestStop();
		}
	});

	connect(addBtn, &QAbstractButton::clicked, this, [=](){
		Q_EMIT requestNewInstrument(TIME);
	});

	connect(removeBtn, &QAbstractButton::clicked, this, &ADCInstrument::requestDeleteInstrument);
}

void ADCInstrument::setupRunSingleButtonHelper()
{
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

void ADCInstrument::addDevice(CollapsableMenuControlButton *b, ToolComponent *dev)
{
	auto devBtn = b;
	m_vcm->add(b);
	QWidget *dev_widget = dynamic_cast<QWidget *>(dev);
	Q_ASSERT(dev_widget);

	channelGroup->addButton(b->getControlBtn());
	QString id = dev->name() + QString::number(uuid++);
	rightStack->add(id, dev_widget);

	connect(b->getControlBtn(), &QPushButton::clicked /* Or ::toggled*/, this, [=](bool b) {
		if(b) {
			tool->requestMenu(channelsMenuId);
			rightStack->show(id);
		}
	});

}

void ADCInstrument::addChannel(MenuControlButton *btn, ToolComponent *ch, CompositeWidget *c)
{
	c->add(btn);
	channelGroup->addButton(btn);

	QString id = ch->name() + QString::number(uuid++);
	QWidget *ch_widget = dynamic_cast<QWidget *>(ch);
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

QPushButton *ADCInstrument::sync() const
{
	return m_sync;
}

void ADCInstrument::stopped()
{
	m_tme->setRunning(false);
	QSignalBlocker sb(m_runBtn);
	m_singleBtn->setChecked(false);
	m_runBtn->setEnabled(true);
	m_runBtn->setChecked(false);
	m_runBtn->setText("Start");
}

void ADCInstrument::started()
{
	m_tme->setRunning(true);
	QSignalBlocker sb(m_runBtn);
	m_runBtn->setChecked(true);
	m_runBtn->setEnabled(true);
	m_runBtn->setText("Stop");
}

VerticalChannelManager *ADCInstrument::vcm() const { return m_vcm; }

ToolTemplate *ADCInstrument::getToolTemplate() { return tool; }

MapStackedWidget *ADCInstrument::getRightStack() { return rightStack; }
