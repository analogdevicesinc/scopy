#include "channelcomponent.h"
#include <plotchannel.h>
#include <plotaxis.h>
#include <widgets/menusectionwidget.h>
#include <widgets/menucollapsesection.h>
#include <widgets/menuplotchannelcurvestylecontrol.h>
#include <QLoggingCategory>
#include <widgets/menuheader.h>
#include <widgets/menuwidget.h>

Q_LOGGING_CATEGORY(CAT_TIME_CHANNELCOMPONENT, "TimeChannelComponent");

using namespace scopy;

ChannelComponent::ChannelComponent(QString ch, QPen pen, QWidget *parent)
	: QWidget(parent)
	, ToolComponent()
	, m_channelName(ch)
	, m_pen(pen)
	, m_chData(new ChannelData(this))
	, m_menu(nullptr)
{

	m_ctrl = nullptr;
	m_name = m_channelName;
	m_enabled = true;
}

ChannelComponent::~ChannelComponent() {}

void ChannelComponent::onStart() {}

void ChannelComponent::onStop() {}

void ChannelComponent::onInit() {}

void ChannelComponent::onDeinit() { m_plotChannelCmpt->deinitPlotComponent(); }

void ChannelComponent::onNewData(const float *xData, const float *yData, size_t size, bool copy) {}

QPen ChannelComponent::pen() const { return m_pen; }

ChannelData *ChannelComponent::chData() const { return m_chData; }

PlotComponentChannel *ChannelComponent::plotChannelCmpt() const { return m_plotChannelCmpt; }

void ChannelComponent::setPlotChannelCmpt(PlotComponentChannel *newPlotChannelCmpt)
{
	m_plotChannelCmpt = newPlotChannelCmpt;
}

MenuControlButton *ChannelComponent::ctrl() { return m_ctrl; }

void ChannelComponent::addChannelToPlot() {}

void ChannelComponent::removeChannelFromPlot() {}

void ChannelComponent::setMenuControlButtonVisible(bool b) { m_ctrl->setVisible(b); }

MenuWidget *ChannelComponent::menu() { return m_menu; }

void ChannelComponent::enable()
{
	m_plotChannelCmpt->enable();
	ToolComponent::enable();
}

void ChannelComponent::disable()
{
	m_plotChannelCmpt->disable();
	ToolComponent::disable();
}

void ChannelComponent::initMenu(QWidget *parent) { m_menu = new MenuWidget(m_channelName, m_pen, parent); }

void ChannelComponent::createMenuControlButton(ChannelComponent *c, QWidget *parent)
{
	c->m_ctrl = new MenuControlButton(parent);
	c->m_ctrl->setName(c->m_channelName);
	c->m_ctrl->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	c->m_ctrl->setOpenMenuChecksThis(true);
	c->m_ctrl->setDoubleClickToOpenMenu(true);
	c->m_ctrl->setColor(c->pen().color());
	c->m_ctrl->button()->setVisible(false);
	c->m_ctrl->setCheckable(true);

	connect(c->m_ctrl->checkBox(), &QCheckBox::toggled, c, [=](bool b) {
		if(b) {
			c->enable();
		} else {
			c->disable();
		}
		c->plotChannelCmpt()->plotComponent()->replot();
	});
	c->m_ctrl->checkBox()->setChecked(true);
	c->setEnabled(true);
}

SamplingInfo ChannelComponent::samplingInfo() { return m_samplingInfo; }

void ChannelComponent::setSamplingInfo(SamplingInfo p) { m_samplingInfo = p; }

#include "moc_channelcomponent.cpp"
