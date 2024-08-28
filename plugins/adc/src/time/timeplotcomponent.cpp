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

#include <gui/widgets/plotinfowidgets.h>

using namespace scopy;
using namespace scopy::adc;
using namespace scopy::gui;

TimePlotComponent::TimePlotComponent(QString name, uint32_t uuid, QWidget *parent)
	: PlotComponent(name, uuid, parent)
	, m_plotMenu(nullptr)
	, m_XYXChannel(nullptr)
	, m_singleYMode(true)
{
	m_timePlot = new PlotWidget(this);
	m_timePlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_timePlot->xAxis()->setInterval(0, 1);
	m_timePlot->xAxis()->setVisible(true);

	m_xyPlot = new PlotWidget(this);
	m_xyPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_xyPlot->xAxis()->setInterval(-2048, 2048);
	m_xyPlot->xAxis()->setVisible(true);

	m_plots.append(m_timePlot);
	m_plots.append(m_xyPlot);

	auto nameLbl = m_timePlot->getPlotInfo()->addLabelInfo(IP_RIGHT);
	nameLbl->setText(m_name);
	connect(this, &PlotComponent::nameChanged, nameLbl, &QLabel::setText);

	m_timePlotInfo = new TimeSamplingInfo(m_timePlot);
	m_timePlot->getPlotInfo()->addCustomInfo(m_timePlotInfo, IP_RIGHT);

	auto m_timeStampInfo = new TimestampInfo(m_timePlot, m_timePlot);
	m_timePlot->getPlotInfo()->addCustomInfo(m_timeStampInfo, IP_RIGHT);

	/*	connect(m_plot->navigator(), &PlotNavigator::rectChanged, this,
		[=]() { m_info->update(m_currentSamplingInfo); });
	*/

	m_plotLayout->addWidget(m_timePlot);
	m_plotLayout->addWidget(m_xyPlot);

	// Need to set this for some reason .. spinboxes should be refactored
	m_timePlot->yAxis()->setUnits("V");

	m_plotMenu = new TimePlotComponentSettings(this, parent);
	addComponent(m_plotMenu);

	connect(m_plotMenu, &TimePlotComponentSettings::requestDeletePlot, this, [=]() { Q_EMIT requestDeletePlot(); });
	m_cursor = new CursorController(m_timePlot, this);
}

TimePlotComponent::~TimePlotComponent() {}

PlotWidget *TimePlotComponent::timePlot() { return m_plots[0]; }
PlotWidget *TimePlotComponent::xyPlot() { return m_plots[1]; }

void TimePlotComponent::setSingleYModeAll(bool b)
{
	m_singleYMode = b;
	for(auto ch : qAsConst(m_channels)) {
		auto pcc = dynamic_cast<TimePlotComponentChannel *>(ch);
		pcc->lockYAxis(b);
	}
}

void TimePlotComponent::showXSourceOnXy(bool b)
{
	m_showXSourceOnXy = b;
	auto xyPlotChCmpt = dynamic_cast<TimePlotComponentChannel *>(m_XYXChannel->plotChannelCmpt());
	xyPlotChCmpt->m_xyPlotCh->setEnabled(b);
}

ChannelComponent *TimePlotComponent::XYXChannel() { return m_XYXChannel; }

void TimePlotComponent::setXYXChannel(ChannelComponent *c)
{
	disconnect(xyDataConn);
	disconnect(xyAxisMinConn);
	disconnect(xyAxisMaxConn);

	if(m_XYXChannel) {
		auto xyPlotChCmpt = dynamic_cast<TimePlotComponentChannel *>(m_XYXChannel->plotChannelCmpt());
		xyPlotChCmpt->m_xyPlotCh->setEnabled(true);
	}
	m_XYXChannel = c;
	if(c) {
		auto cPlotChCmpt = dynamic_cast<TimePlotComponentChannel *>(c->plotChannelCmpt());
		onXyXNewData(c->chData()->xData(), c->chData()->yData(), c->chData()->size(), true);
		xyDataConn = connect(c->chData(), &ChannelData::newData, this, &TimePlotComponent::onXyXNewData);
		cPlotChCmpt->m_xyPlotCh->setEnabled(m_showXSourceOnXy);
		xyAxisMinConn = connect(cPlotChCmpt->m_timePlotYAxis, &PlotAxis::minChanged, this, [=](double val) {
			if(!cPlotChCmpt->m_singleYMode) {
				m_xyPlot->xAxis()->setMin(val);
			}
		});
		xyAxisMaxConn = connect(cPlotChCmpt->m_timePlotYAxis, &PlotAxis::maxChanged, this, [=](double val) {
			if(!cPlotChCmpt->m_singleYMode) {
				m_xyPlot->xAxis()->setMax(val);
			}
		});
	}
}

void TimePlotComponent::refreshXYXAxis()
{
	double min = m_xyPlot->yAxis()->min();
	double max = m_xyPlot->yAxis()->max();

	if(m_XYXChannel) {
		auto xyPlotChCmpt = dynamic_cast<TimePlotComponentChannel *>(m_XYXChannel->plotChannelCmpt());
		if(!xyPlotChCmpt->m_singleYMode) {
			min = xyPlotChCmpt->m_timePlotYAxis->min();
			max = xyPlotChCmpt->m_timePlotYAxis->max();
		}
	}

	m_xyPlot->xAxis()->setInterval(min, max);
}

void TimePlotComponent::onXyXNewData(const float *xData_, const float *yData_, size_t size, bool copy)
{
	xyXData = yData_;
	for(PlotComponentChannel *ch : qAsConst(m_channels)) {
		auto pcc = dynamic_cast<TimePlotComponentChannel *>(ch);
		pcc->setXyXData(xyXData);
		pcc->refreshData(copy);
	}
}

TimeSamplingInfo *TimePlotComponent::timePlotInfo() const { return m_timePlotInfo; }

void TimePlotComponent::refreshXYXData()
{
	for(PlotComponentChannel *ch : qAsConst(m_channels)) {
		auto pcc = dynamic_cast<TimePlotComponentChannel *>(ch);
		pcc->setXyXData(xyXData);
		pcc->refreshData(true);
	}
}

void TimePlotComponent::addChannel(ChannelComponent *c)
{
	PlotComponent::addChannel(c);
	if(m_XYXChannel == nullptr) {
		// if we don't have an XY channel, set this one
		setXYXChannel(c);
	}
	refreshXYXData();
	m_plotMenu->addChannel(c);
}

void TimePlotComponent::selectChannel(ChannelComponent *c)
{

	m_timePlot->selectChannel(dynamic_cast<TimePlotComponentChannel *>(c->plotChannelCmpt())->m_timePlotCh);
	if(m_XYXChannel != c || m_showXSourceOnXy) {
		m_xyPlot->selectChannel(dynamic_cast<TimePlotComponentChannel *>(c->plotChannelCmpt())->m_xyPlotCh);
	}
}

void TimePlotComponent::removeChannel(ChannelComponent *c)
{
	PlotComponent::removeChannel(c);

	if(m_XYXChannel == c) {
		if(m_channels.size() > 0) {
			setXYXChannel(m_channels[0]->channelComponent());
		} else {
			setXYXChannel(nullptr);
		}
	}
	m_plotMenu->removeChannel(c);
}

void TimePlotComponent::setXInterval(QPair<double, double> p) { setXInterval(p.first, p.second); }

void TimePlotComponent::setXInterval(double min, double max)
{
	for(auto plt : qAsConst(m_plots)) {
		timePlot()->xAxis()->setInterval(min, max);
	}
}

bool TimePlotComponent::singleYMode() const { return m_singleYMode; }

TimePlotComponentSettings *TimePlotComponent::createPlotMenu(QWidget *parent) { return m_plotMenu; }

TimePlotComponentSettings *TimePlotComponent::plotMenu() { return m_plotMenu; }
