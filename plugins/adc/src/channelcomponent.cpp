#include "channelcomponent.h"
#include <gui/plotchannel.h>
#include <gui/plotaxis.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <QLoggingCategory>
#include <timeplotcomponentchannel.h>

Q_LOGGING_CATEGORY(CAT_TIME_CHANNELCOMPONENT, "TimeChannelComponent");


using namespace scopy;
using namespace gui;
using namespace scopy::adc;
ChannelComponent::ChannelComponent(QString ch, TimePlotComponent *m_plot, QPen pen, QWidget *parent)
	: QWidget(parent)
	, ToolComponent()
	, m_channelName(ch)
	, m_pen(pen)
	, m_chData(new ChannelData(this))
	, m_plotChannelCmpt(new TimePlotComponentChannel(this,m_plot,this))
{

	connect(m_chData, &ChannelData::newData,m_plotChannelCmpt,&TimePlotComponentChannel::onNewData);
	m_name = m_channelName;
	m_enabled = true;
}

ChannelComponent::~ChannelComponent() {}

QWidget *ChannelComponent::getMenuControlWidget() { return m_mcw; }

void ChannelComponent::setMenuControlWidget(QWidget *w) { m_mcw = w; }

void ChannelComponent::onStart() {}

void ChannelComponent::onStop() {}

void ChannelComponent::onInit() {}

void ChannelComponent::onDeinit() {}


QPen ChannelComponent::pen() const { return m_pen; }

ChannelData *ChannelComponent::chData() const
{
	return m_chData;
}

TimePlotComponentChannel *ChannelComponent::plotChannelCmpt() const
{
	return m_plotChannelCmpt;
}

void ChannelComponent::setPlotChannelCmpt(TimePlotComponentChannel *newPlotChannelCmpt)
{
	m_plotChannelCmpt = newPlotChannelCmpt;
}

