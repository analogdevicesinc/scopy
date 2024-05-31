#include "timeplotcomponentchannel.h"
#include <pluginbase/preferences.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <gui/widgets/menusectionwidget.h>

using namespace scopy;
using namespace scopy::adc;

TimePlotComponentChannel::TimePlotComponentChannel(ChannelComponent *ch, TimePlotComponent *plotComponent, QObject *parent)
	: QObject(parent)
	, m_enabled(true)
{
	auto timeplot = plotComponent->timePlot();
	auto xyplot = plotComponent->xyPlot();

	m_ch = ch;
	initPlotComponent(plotComponent);

	m_timePlotYAxis->setUnits("V");
	m_timePlotCh->xAxis()->setUnits("s");
	m_timePlotYAxis->setInterval(-2048,2048);
	m_xyPlotYAxis->setInterval(-2048,2048);

}

void TimePlotComponentChannel::deinitPlotComponent() {
	if(m_plotComponent == nullptr)
		return;

	auto timeplot = m_plotComponent->timePlot();
	auto xyplot = m_plotComponent->xyPlot();

	timeplot->removePlotAxisHandle(m_timePlotAxisHandle);
	timeplot->removePlotChannel(m_timePlotCh);
	xyplot->removePlotChannel(m_xyPlotCh);

	delete m_timePlotYAxis;
	delete m_timePlotCh;
	delete m_timePlotAxisHandle;
	delete m_xyPlotYAxis;
	delete m_xyPlotCh;

}

void TimePlotComponentChannel::initPlotComponent(TimePlotComponent *plotComponent) {

	auto timeplot = plotComponent->timePlot();
	auto xyplot = plotComponent->xyPlot();

	if(plotComponent != m_plotComponent) {
		deinitPlotComponent();
	}

	m_plotComponent = plotComponent;

	int yPlotAxisPosition = Preferences::get("adc_plot_yaxis_label_position").toInt();
	int yPlotAxisHandle = Preferences::get("adc_plot_yaxis_handle_position").toInt();
	m_timePlotYAxis = new PlotAxis(yPlotAxisPosition, timeplot, m_ch->pen(), this);
	m_timePlotCh = new PlotChannel(m_ch->name(), m_ch->pen(), timeplot->xAxis(), m_timePlotYAxis, this);
	m_timePlotAxisHandle = new PlotAxisHandle(timeplot, m_timePlotYAxis);

	m_timePlotAxisHandle->handle()->setHandlePos((HandlePos)yPlotAxisHandle);
	m_timePlotAxisHandle->handle()->setBarVisibility(BarVisibility::ON_HOVER);
	m_timePlotAxisHandle->handle()->setColor(m_ch->pen().color());

	connect(m_timePlotAxisHandle, &PlotAxisHandle::scalePosChanged, this, [=](double pos) {
		double min = m_timePlotYAxis->min() - pos;
		double max = m_timePlotYAxis->max() - pos;
		m_timePlotYAxis->setInterval(min, max);
		m_plotComponent->replot();
	});

	m_timePlotCh->setHandle(m_timePlotAxisHandle);
	timeplot->addPlotAxisHandle(m_timePlotAxisHandle);
	timeplot->addPlotChannel(m_timePlotCh);
	m_timePlotCh->setEnabled(true);

	m_xyPlotYAxis = new PlotAxis(yPlotAxisPosition, xyplot, m_ch->pen(), this);
	m_xyPlotCh = new PlotChannel(m_ch->name(), m_ch->pen(), xyplot->xAxis(), m_xyPlotYAxis, this);
	xyplot->addPlotChannel(m_xyPlotCh);
	m_xyPlotCh->setEnabled(true);

	setSingleYMode(m_plotComponent->singleYMode());
	m_timePlotYAxis->setInterval(-2048,2048);
	m_xyPlotYAxis->setInterval(-2048,2048);
}

TimePlotComponentChannel::~TimePlotComponentChannel() {

}


void TimePlotComponentChannel::refreshData(bool copy) {
	auto data = m_ch->chData();
	m_timePlotCh->setSamples(data->xData(), data->yData(),data->size(), copy);
	if(m_xyXData) {
		m_xyPlotCh->setSamples(m_xyXData,data->yData(),data->size(), copy);
	}
}

void TimePlotComponentChannel::onNewData(const float *xData_, const float *yData_, size_t size, bool copy) {
	refreshData(copy);
}

void TimePlotComponentChannel::setXyXData(const float *xyxdata) {
	m_xyXData = xyxdata;
}

void TimePlotComponentChannel::setSingleYMode(bool b) {
	m_singleYMode = b;
	if(m_singleYMode) {
		QwtAxisId id = m_plotComponent->timePlot()->yAxis()->axisId();
		m_timePlotCh->curve()->setYAxis(id);
		m_xyPlotCh->curve()->setYAxis(m_plotComponent->xyPlot()->yAxis()->axisId());
	} else {
		QwtAxisId id = m_timePlotYAxis->axisId();
		m_timePlotCh->curve()->setYAxis(id);
		m_xyPlotCh->curve()->setYAxis(id);
	}
	m_timePlotAxisHandle->setVisible(!b);
}

QWidget *TimePlotComponentChannel::createCurveMenu(QWidget *parent) {

	MenuSectionWidget *curvecontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *curve = new MenuCollapseSection("CURVE", MenuCollapseSection::MHCW_NONE, curvecontainer);

	MenuPlotChannelCurveStyleControl *curveSettings = new MenuPlotChannelCurveStyleControl(curve);
	curveSettings->addChannels(m_timePlotCh);

	curve->contentLayout()->addWidget(curveSettings);
	curvecontainer->contentLayout()->addWidget(curve);

	return curvecontainer;
}

void TimePlotComponentChannel::enable() {
	m_timePlotCh->enable();
	m_xyPlotCh->enable();
	if(m_timePlotAxisHandle) {
		m_timePlotAxisHandle->handle()->setVisible(true);
		m_timePlotAxisHandle->handle()->raise();
	}
	m_enabled = true;
}

void TimePlotComponentChannel::disable() {
	m_timePlotCh->disable();
	m_xyPlotCh->disable();
	if(m_timePlotAxisHandle) {
		m_timePlotAxisHandle->handle()->setVisible(false);
	}
	m_enabled = false;
}

