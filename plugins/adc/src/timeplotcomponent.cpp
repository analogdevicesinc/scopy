#include "timeplotcomponent.h"
#include "plotaxis.h"

#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include "channelcomponent.h"
#include <pluginbase/preferences.h>
#include <timeplotcomponentchannel.h>
#include <qwt_point_data.h>
#include <QLineEdit>
#include <timeplotcomponentsettings.h>

using namespace scopy;
using namespace scopy::adc;
using namespace scopy::gui;

TimePlotComponent::TimePlotComponent(QString name, uint32_t uuid, QWidget *parent)
	: QWidget(parent)
	, MetaComponent()
	, m_uuid(uuid)
	, m_plotMenu(nullptr)
	, m_XYXChannel(nullptr)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_plotLayout = new QHBoxLayout(this);
	m_plotLayout->setMargin(0);
	m_plotLayout->setSpacing(0);
	setLayout(m_plotLayout);
	m_name = name;

	m_timePlot = new PlotWidget(this);
	m_timePlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_timePlot->xAxis()->setInterval(0, 1);
	m_timePlot->xAxis()->setVisible(true);

	/*m_timeInfo = new PlotInfo(m_timePlot, this);
	m_timePlot->addPlotInfoSlot(m_timeInfo);*/

	m_xyPlot = new PlotWidget(this);
	m_xyPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_xyPlot->xAxis()->setInterval(-2048, 2048);
	m_xyPlot->xAxis()->setVisible(true);

	/*	connect(m_plot->navigator(), &PlotNavigator::rectChanged, this,
		[=]() { m_info->update(m_currentSamplingInfo); });
	*/

	m_plotLayout->addWidget(m_timePlot);
	m_plotLayout->addWidget(m_xyPlot);

	// Need to set this for some reason .. spinboxes should be refactored
	m_timePlot->yAxis()->setUnits("V");

	m_plotMenu = new TimePlotComponentSettings(this, parent);
	addComponent(m_plotMenu);
}

TimePlotComponent::~TimePlotComponent() {}

PlotWidget *TimePlotComponent::timePlot() { return m_timePlot; }
PlotWidget *TimePlotComponent::xyPlot() { return m_xyPlot; }

void TimePlotComponent::replot()
{
	m_timePlot->replot();
	m_xyPlot->replot();
}

void TimePlotComponent::showPlotLabels(bool b)
{
	m_timePlot->setShowXAxisLabels(b);
	m_timePlot->setShowYAxisLabels(b);

	m_xyPlot->setShowXAxisLabels(b);
	m_xyPlot->setShowYAxisLabels(b);

	m_timePlot->showAxisLabels();
	m_xyPlot->showAxisLabels();
}

void TimePlotComponent::setSingleYMode(bool b)
{
	m_singleYMode = b;
	for(TimePlotComponentChannel *pcc : qAsConst(m_channels)) {
		pcc->setSingleYMode(b);
	}
}

void TimePlotComponent::showXSourceOnXy(bool b)
{
	m_showXSourceOnXy = b;
	m_XYXChannel->plotChannelCmpt()->m_xyPlotCh->setEnabled(b);
}

void TimePlotComponent::setName(QString s) {
	m_name = s;
	Q_EMIT nameChanged(s);
}

ChannelComponent *TimePlotComponent::XYXChannel() { return m_XYXChannel; }

void TimePlotComponent::onStart() { MetaComponent::onStart(); }

void TimePlotComponent::onStop() { MetaComponent::onStop(); }

void TimePlotComponent::onInit() {}

void TimePlotComponent::onDeinit() {}

void TimePlotComponent::setXYXChannel(ChannelComponent *c)
{
	disconnect(xyDataConn);
	if(m_XYXChannel) {
		m_XYXChannel->plotChannelCmpt()->m_xyPlotCh->setEnabled(true);
	}
	m_XYXChannel = c;
	if(c) {
		xyDataConn = connect(c->chData(), &ChannelData::newData, this,
				     [=](const float *xData_, const float *yData_, size_t size, bool copy) {
					     xyXData = yData_;
					     for(TimePlotComponentChannel *ch : qAsConst(m_channels)) {
						     ch->setXyXData(xyXData);
					     }
				     });
		m_XYXChannel->plotChannelCmpt()->m_xyPlotCh->setEnabled(m_showXSourceOnXy);
	}
}

void TimePlotComponent::addChannel(ChannelComponent *c)
{
	m_channels.append(c->plotChannelCmpt());
	if(m_XYXChannel == nullptr) {
		setXYXChannel(c);
	}
	m_plotMenu->addChannel(c);
}

void TimePlotComponent::removeChannel(ChannelComponent *c)
{
	TimePlotComponentChannel *toRemove;
	for(TimePlotComponentChannel *ch : qAsConst(m_channels)) {
		if(ch->m_ch == c) {
			toRemove = ch;
			break;
		}
	}
	m_channels.removeAll(toRemove);

	if(m_XYXChannel == c) {
		if(m_channels.size() > 0) {
			setXYXChannel(m_channels[0]->m_ch);
		} else {
			setXYXChannel(nullptr);
		}
	}
	m_plotMenu->removeChannel(c);
}

bool TimePlotComponent::singleYMode() const { return m_singleYMode; }

TimePlotComponentSettings *TimePlotComponent::createPlotMenu(QWidget *parent) { return m_plotMenu; }

TimePlotComponentSettings *TimePlotComponent::plotMenu() { return m_plotMenu; }

uint32_t TimePlotComponent::uuid() { return m_uuid; }
