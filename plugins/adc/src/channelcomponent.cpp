#include "channelcomponent.h"
#include <gui/plotchannel.h>
#include <gui/plotaxis.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_TIME_CHANNELCOMPONENT, "TimeChannelComponent");


using namespace scopy;
using namespace gui;
using namespace scopy::adc;
ChannelComponent::ChannelComponent(QString ch, PlotComponent *m_plot, QPen pen, QWidget *parent)
	: QWidget(parent)
	, m_channelName(ch)
	, m_pen(pen)
	, m_enabled(true)
	, m_chData(new ChannelData(this))
	, m_plotChannelCmpt(new PlotChannelComponent(this,m_plot,this))
{

	connect(m_chData, &ChannelData::newData,m_plotChannelCmpt,&PlotChannelComponent::onNewData);

	m_name = m_channelName;
}

ChannelComponent::~ChannelComponent() {}

QWidget *ChannelComponent::getMenuControlWidget() { return m_mcw; }

void ChannelComponent::setMenuControlWidget(QWidget *w) { m_mcw = w; }

void ChannelComponent::onStart() {}

void ChannelComponent::onStop() {}

void ChannelComponent::onInit() {}

void ChannelComponent::onDeinit() {}


QPen ChannelComponent::pen() const { return m_pen; }

bool ChannelComponent::enabled() const { return m_enabled; }

ChannelData *ChannelComponent::chData() const
{
	return m_chData;
}

PlotChannelComponent *ChannelComponent::plotChannelCmpt() const
{
	return m_plotChannelCmpt;
}

void ChannelComponent::setPlotChannelCmpt(PlotChannelComponent *newPlotChannelCmpt)
{
	m_plotChannelCmpt = newPlotChannelCmpt;
}

void ChannelComponent::enableChannel()
{
	qInfo(CAT_TIME_CHANNELCOMPONENT) << m_channelName << " enabled";
	m_enabled = true;
	/**/
	//	m_grch->setEnabled(true);
	//	m_plotAddon->plot()->replot();
}

void ChannelComponent::disableChannel()
{
	qInfo(CAT_TIME_CHANNELCOMPONENT) << m_channelName << " disabled";
	m_enabled = false;
	/**/

	//	m_grch->setEnabled(false);
	//	m_plotAddon->plot()->replot();
}
