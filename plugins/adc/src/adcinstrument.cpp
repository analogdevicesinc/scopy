#include "adcinstrument.h"
#include "stylehelper.h"

#include <gui/widgets/toolbuttons.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/verticalchannelmanager.h>

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
	lay->addWidget(tool);

	openLastMenuBtn = new OpenLastMenuBtn(dynamic_cast<MenuAnim*>(tool->rightContainer()),true,this);
	QButtonGroup* rightMenuBtnGrp = dynamic_cast<OpenLastMenuBtn*>(openLastMenuBtn)->getButtonGroup();
	tool->setLeftContainerWidth(200);
	tool->setRightContainerWidth(300);

	GearBtn *settingsMenu = new GearBtn(this);
	RunBtn *runBtn = new RunBtn(this);
	InfoBtn *infoBtn = new InfoBtn(this);
	SingleShotBtn *singleBtn = new SingleShotBtn(this);
	PrintBtn *printBtn = new PrintBtn(this);

	MenuControlButton *channelsBtn = new MenuControlButton(this);
	channelsBtn->setName("Channels");
	channelsBtn->checkBox()->setVisible(false);
	channelsBtn->button()->setVisible(false);
	channelsBtn->setChecked(true);

	MenuControlButton *cursor = new MenuControlButton(this);
	cursor->setName("Cursors");
	cursor->setCheckBoxStyle(MenuControlButton::CS_SQUARE);
	MenuControlButton *measure = new MenuControlButton(this);
	measure->setName("Measure");

	tool->addWidgetToTopContainerMenuControlHelper(openLastMenuBtn,TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsMenu,TTA_LEFT);

	tool->addWidgetToTopContainerHelper(runBtn,TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(singleBtn,TTA_RIGHT);

	tool->addWidgetToTopContainerHelper(infoBtn,TTA_LEFT);
	tool->addWidgetToTopContainerHelper(printBtn,TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(channelsBtn, TTA_LEFT);

	tool->addWidgetToBottomContainerHelper(cursor, TTA_RIGHT);
	tool->addWidgetToBottomContainerHelper(measure, TTA_RIGHT);


	connect(channelsBtn, &QPushButton::toggled, dynamic_cast<MenuAnim*>(tool->leftContainer()), &MenuAnim::toggleMenu);

	GRTimePlotAddon* plotAddon = dynamic_cast<GRTimePlotAddon*>(proxy->getPlotAddon());
	tool->addWidgetToCentralContainerHelper(plotAddon->getWidget());

	GRTimePlotAddonSettings* plotAddonSettings = dynamic_cast<GRTimePlotAddonSettings*>(proxy->getPlotSettings());
	rightMenuBtnGrp->addButton(settingsMenu);
	QString settingsMenuId = plotAddonSettings->getName() + QString(uuid++);
	tool->rightStack()->add(settingsMenuId, plotAddonSettings->getWidget());
	connect(settingsMenu, &QPushButton::toggled, this, [=](bool b) {if(b) tool->requestMenu(settingsMenuId);});

	VerticalChannelManager *vcm = new VerticalChannelManager(this);
	tool->leftStack()->add("vcm",vcm);

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

			btn->setName(ch->getName());
			btn->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
			btn->setColor(ch->pen().color());
			btn->setCheckable(true);

			QString id = ch->getName() + QString::number(uuid++);
			tool->rightStack()->add(id, ch->getWidget());
			connect(btn->checkBox(), &QCheckBox::toggled, this, [=](bool b) { if(b) ch->enable(); else ch->disable();} );
			connect(btn->button(), &QPushButton::toggled, this, [=](bool b) { if(b) tool->requestMenu(id);});
			connect(btn, &QAbstractButton::toggled, this, [=](){plotAddon->plot()->selectChannel(ch->plotCh());});
			rightMenuBtnGrp->addButton(btn->button());
			plotAddon->onChannelAdded(ch);
			btn->checkBox()->setChecked(true);

		}
	}

	connect(runBtn,&QPushButton::toggled, this, &AdcInstrument::run);
	tool->requestMenu("voltage02");
}

void AdcInstrument::run(bool b) {
	qInfo()<<b;
	QElapsedTimer tim;
	tim.start();


	qInfo()<<tim.elapsed();
	for(auto ch : proxy->getChannelAddons()) {
		if(b)
			ch->onStart();
		else
			ch->onStop();
	}
	qInfo()<<tim.elapsed();
	for(auto dev : proxy->getDeviceAddons()) {
		if(b)
			dev->onStart();
		else
			dev->onStop();
	}
	qInfo()<<tim.elapsed();

	if(b)
		proxy->getPlotAddon()->onStart();
	else
		proxy->getPlotAddon()->onStop();
	qInfo()<<tim.elapsed();



}





