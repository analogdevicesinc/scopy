#include "channelcomponent.h"
#include <gui/plotchannel.h>
#include <gui/plotaxis.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>

Q_LOGGING_CATEGORY(CAT_TIME_CHANNELCOMPONENT, "TimeChannelComponent");


using namespace scopy;
using namespace gui;
using namespace scopy::adc;
ChannelComponent::ChannelComponent(QString ch, PlotComponent *plotComponent, QPen pen, QWidget *parent)
	: QWidget(parent)
	, m_channelName(ch)
	, m_plotComponent(plotComponent)
	, m_pen(pen)
{}

ChannelComponent::~ChannelComponent() {}

QWidget *ChannelComponent::getMenuControlWidget() { return m_mcw; }

void ChannelComponent::setMenuControlWidget(QWidget *w) { m_mcw = w; }

void ChannelComponent::onStart() {}

void ChannelComponent::onStop() {}

void ChannelComponent::onInit() {}

void ChannelComponent::onDeinit() {}

void ChannelComponent::onNewData(const float *xData, const float *yData, int size) {}

PlotChannel *ChannelComponent::plotCh() const { return m_plotCh; }

QPen ChannelComponent::pen() const { return m_pen; }

bool ChannelComponent::enabled() const { return m_enabled; }

void ChannelComponent::enable()
{
	qInfo(CAT_TIME_CHANNELCOMPONENT) << m_channelName << " enabled";
	m_enabled = true;
	m_plotCh->enable();
	m_plotAxisHandle->handle()->setVisible(true);
	m_plotAxisHandle->handle()->raise();
	//	m_grch->setEnabled(true);
	m_plotComponent->replot();
	//	m_plotAddon->plot()->replot();
}

void ChannelComponent::disable()
{
	qInfo(CAT_TIME_CHANNELCOMPONENT) << m_channelName << " disabled";
	m_enabled = false;
	m_plotCh->disable();
	m_plotAxisHandle->handle()->setVisible(false);

	       //	m_grch->setEnabled(false);
	m_plotComponent->replot();
	//	m_plotAddon->plot()->replot();
}

QWidget *ChannelComponent::createCurveMenu(QWidget *parent)
{

	MenuSectionWidget *curvecontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *curve = new MenuCollapseSection("CURVE", MenuCollapseSection::MHCW_NONE, curvecontainer);

	MenuPlotChannelCurveStyleControl *curveSettings = new MenuPlotChannelCurveStyleControl(curve);
	curveSettings->addChannels(m_plotCh);

	curve->contentLayout()->addWidget(curveSettings);
	curvecontainer->contentLayout()->addWidget(curve);

	return curvecontainer;
}

