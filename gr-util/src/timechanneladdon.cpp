#include "timechanneladdon.h"

#include "grtimeplotaddon.h"
#include "plotaxishandle.h"

#include <QLoggingCategory>

#include <menucollapsesection.h>
#include <menucombo.h>
#include <menusectionwidget.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>

Q_LOGGING_CATEGORY(CAT_TIME_CHANNEL, "TimeChannelAddon");

using namespace scopy;
using namespace scopy::gui;
using namespace scopy::grutil;

ChannelAddon::ChannelAddon(QString ch, PlotAddon *plotAddon, QPen pen, QObject *parent)
	: QObject(parent)
	, m_channelName(ch)
	, m_plotAddon(plotAddon)
	, m_pen(pen)
{}

ChannelAddon::~ChannelAddon() {}

QString ChannelAddon::getName() { return m_channelName; }

QWidget *ChannelAddon::getWidget() { return widget; }

QWidget *ChannelAddon::getMenuControlWidget() { return m_mcw; }

void ChannelAddon::setMenuControlWidget(QWidget *w) { m_mcw = w; }

void ChannelAddon::onStart() {}

void ChannelAddon::onStop() {}

void ChannelAddon::onInit() {}

void ChannelAddon::onDeinit() {}

void ChannelAddon::onNewData(const float *xData, const float *yData, int size) {}

PlotChannel *ChannelAddon::plotCh() const { return m_plotCh; }

QPen ChannelAddon::pen() const { return m_pen; }

bool ChannelAddon::enabled() const { return m_enabled; }

void ChannelAddon::enable()
{
	qInfo(CAT_TIME_CHANNEL) << m_channelName << " enabled";
	m_enabled = true;
	m_plotCh->enable();
	m_plotAxisHandle->handle()->setVisible(true);
	m_plotAxisHandle->handle()->raise();
	//	m_grch->setEnabled(true);
	m_plotAddon->replot();
	//	m_plotAddon->plot()->replot();
}

void ChannelAddon::disable()
{
	qInfo(CAT_TIME_CHANNEL) << m_channelName << " disabled";
	m_enabled = false;
	m_plotCh->disable();
	m_plotAxisHandle->handle()->setVisible(false);

	//	m_grch->setEnabled(false);
	m_plotAddon->replot();
	//	m_plotAddon->plot()->replot();
}

QWidget *ChannelAddon::createCurveMenu(QWidget *parent)
{

	MenuSectionWidget *curvecontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *curve = new MenuCollapseSection("CURVE", MenuCollapseSection::MHCW_NONE, curvecontainer);

	MenuPlotChannelCurveStyleControl *curveSettings = new MenuPlotChannelCurveStyleControl(curve);
	curveSettings->addChannels(m_plotCh);

	curve->contentLayout()->addWidget(curveSettings);
	curvecontainer->contentLayout()->addWidget(curve);

	return curvecontainer;
}

TimeChannelAddon::TimeChannelAddon(QString ch, PlotAddon *plotAddon, QPen pen, QObject *parent)
	: ChannelAddon(ch, plotAddon, pen, parent)
{}

TimeChannelAddon::~TimeChannelAddon() {}
