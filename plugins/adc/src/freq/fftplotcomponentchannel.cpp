#include "fftplotcomponentchannel.h"
#include <pluginbase/preferences.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <gui/widgets/menusectionwidget.h>

using namespace scopy;
using namespace scopy::adc;

FFTPlotComponentChannel::FFTPlotComponentChannel(ChannelComponent *ch, FFTPlotComponent *plotComponent,
						   QObject *parent)
	: QObject(parent)
	, m_enabled(true)
{
	auto fftplot = plotComponent->fftPlot();

	m_ch = ch;
	m_plotComponent = nullptr;
	initPlotComponent(plotComponent);

	m_fftPlotYAxis->setUnits("V");
	m_fftPlotCh->xAxis()->setUnits("s");
	m_fftPlotYAxis->setInterval(-2048, 2048);
}

void FFTPlotComponentChannel::deinitPlotComponent()
{
	if(m_plotComponent == nullptr)
		return;

	auto fftplot = m_plotComponent->fftPlot();
	fftplot->removePlotAxisHandle(m_fftPlotAxisHandle);
	fftplot->removePlotChannel(m_fftPlotCh);

	delete m_fftPlotYAxis;
	delete m_fftPlotCh;
	delete m_fftPlotAxisHandle;
}

void FFTPlotComponentChannel::initPlotComponent(PlotComponent *pc)
{
	FFTPlotComponent* plotComponent = dynamic_cast<FFTPlotComponent*>(pc);
	auto fftplot = plotComponent->fftPlot();

	if(plotComponent != m_plotComponent) {
		deinitPlotComponent();
	}

	m_plotComponent = plotComponent;

	int yPlotAxisPosition = Preferences::get("adc_plot_yaxis_label_position").toInt();
	int yPlotAxisHandle = Preferences::get("adc_plot_yaxis_handle_position").toInt();
	m_fftPlotYAxis = new PlotAxis(yPlotAxisPosition, fftplot, m_ch->pen(), this);
	m_fftPlotYAxis->setUnits("dB");

	m_fftPlotCh = new PlotChannel(m_ch->name(), m_ch->pen(), fftplot->xAxis(), m_fftPlotYAxis, this);
	m_fftPlotAxisHandle = new PlotAxisHandle(fftplot, m_fftPlotYAxis);

	m_fftPlotAxisHandle->handle()->setHandlePos((HandlePos)yPlotAxisHandle);
	m_fftPlotAxisHandle->handle()->setBarVisibility(BarVisibility::ON_HOVER);
	m_fftPlotAxisHandle->handle()->setColor(m_ch->pen().color());

	connect(m_fftPlotAxisHandle, &PlotAxisHandle::scalePosChanged, this, [=](double pos) {
		double min = m_fftPlotYAxis->min() - pos;
		double max = m_fftPlotYAxis->max() - pos;
		m_fftPlotYAxis->setInterval(min, max);
		m_plotComponent->replot();
	});

	m_fftPlotCh->setHandle(m_fftPlotAxisHandle);
	fftplot->addPlotAxisHandle(m_fftPlotAxisHandle);
	fftplot->addPlotChannel(m_fftPlotCh);
	m_fftPlotCh->setEnabled(true);

	lockYAxis(true);
	m_fftPlotYAxis->setInterval(-2048, 2048);
	refreshData(true);
}

FFTPlotComponentChannel::~FFTPlotComponentChannel() {}

void FFTPlotComponentChannel::refreshData(bool copy)
{
	auto data = m_ch->chData();
	m_fftPlotCh->setSamples(data->xData(), data->yData(), data->size(), copy);
}

void FFTPlotComponentChannel::onNewData(const float *xData_, const float *yData_, size_t size, bool copy)
{
	refreshData(copy);
}


void FFTPlotComponentChannel::lockYAxis(bool b)
{
	m_singleYMode = b;
	if(m_singleYMode) {
		PlotAxis *time = m_plotComponent->fftPlot()->yAxis();
		m_plotComponent->fftPlot()->plotChannelChangeYAxis(m_fftPlotCh, time);
	} else {
		PlotAxis *time = m_fftPlotYAxis;
		m_plotComponent->fftPlot()->plotChannelChangeYAxis(m_fftPlotCh, time);
	}


	m_fftPlotAxisHandle->handle()->setVisible(!b);
	m_plotComponent->refreshAxisLabels();
	m_plotComponent->replot();
}

QWidget *FFTPlotComponentChannel::createCurveMenu(QWidget *parent)
{

	MenuSectionCollapseWidget *curve =
		new MenuSectionCollapseWidget("CURVE", MenuCollapseSection::MHCW_NONE, parent);

	MenuPlotChannelCurveStyleControl *curveSettings = new MenuPlotChannelCurveStyleControl(curve);
	curveSettings->addChannels(m_fftPlotCh);

	curve->contentLayout()->addWidget(curveSettings);

	return curve;
}

ChannelComponent *FFTPlotComponentChannel::channelComponent()
{
	return m_ch;
}

PlotComponent *FFTPlotComponentChannel::plotComponent()
{
	return m_plotComponent;
}

PlotChannel *FFTPlotComponentChannel::plotChannel()
{
	return m_fftPlotCh;
}

void FFTPlotComponentChannel::enable()
{
	m_fftPlotCh->enable();
	if(m_fftPlotAxisHandle && !m_singleYMode) {
		m_fftPlotAxisHandle->handle()->setVisible(true);
		m_fftPlotAxisHandle->handle()->raise();
	}
	m_enabled = true;
}

void FFTPlotComponentChannel::disable()
{
	m_fftPlotCh->disable();
	if(m_fftPlotAxisHandle) {
		m_fftPlotAxisHandle->handle()->setVisible(false);
	}
	m_enabled = false;
}
