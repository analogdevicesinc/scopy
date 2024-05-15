#include "plotcomponent.h"
#include "menucollapsesection.h"
#include "plotaxis.h"

#include "menusectionwidget.h"
#include "widgets/menuplotchannelcurvestylecontrol.h"
#include "channelcomponent.h"

#include <qwt_point_data.h>


using namespace scopy;
using namespace scopy::adc;
using namespace scopy::gui;

PlotComponent::PlotComponent(QString name, QWidget *parent)
	: QWidget(parent)
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

	m_timeInfo = new TimePlotInfo(m_timePlot, this);
	m_timePlot->addPlotInfoSlot(m_timeInfo);


	m_xyPlot = new PlotWidget(this);
	m_xyPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_xyPlot->xAxis()->setInterval(-2048, 2048);
	m_xyPlot->xAxis()->setVisible(true);
	//	m_plotWidget->topHandlesArea()->setVisible(true);

	/*	connect(m_plot->navigator(), &PlotNavigator::rectChanged, this,
		[=]() { m_info->update(m_currentSamplingInfo); });
	*/

	m_plotLayout->addWidget(m_timePlot);
	m_plotLayout->addWidget(m_xyPlot);
}

PlotComponent::~PlotComponent() {}

PlotWidget *PlotComponent::timePlot() { return m_timePlot; }
PlotWidget *PlotComponent::xyPlot() { return m_xyPlot; }

void PlotComponent::replot() { m_timePlot->replot();  m_xyPlot->replot(); }

void PlotComponent::onStart() {}

void PlotComponent::onStop() {}

void PlotComponent::onInit() {}

void PlotComponent::onDeinit() {}

void PlotComponent::setXYXChannel(ChannelComponent *c) {
	disconnect(xyDataConn);

	xyDataConn =  connect(c->chData(), &ChannelData::newData,this, [=](const float* xData_, const float* yData_, size_t size, bool copy){
		xyXData = yData_;
		// just set xyxdata to plotchannel and let replot handle it ?
		for(PlotChannelComponent *ch : qAsConst(m_channels)) {
			ch->setXyXData(xyXData);
			//ch->refreshData(copy);
		}


	});
}

void PlotComponent::addChannel(ChannelComponent *c)
{
	m_channels.append(c->plotChannelCmpt());
	setXYXChannel(c);
	/*m_channels.append(new PlotChannelComponent(c,this,this));
	c->setPlotChannelCmpt()*/
}

void PlotComponent::removeChannel(ChannelComponent *c)
{
	/*PlotChannelComponent* toRemove;
	for(PlotChannelComponent* ch : qAsConst(m_channels)) {
		if(ch->m_ch == c) {
			toRemove = ch;
			break;
		}
	}
	m_channels.removeAll(toRemove);*/

}

QWidget *PlotComponent::createPlotMenu()
{
	QWidget *w = nullptr;
	return w;
}


PlotChannelComponent::PlotChannelComponent(ChannelComponent *ch, PlotComponent *plotComponent, QObject *parent) : QObject(parent) { // is this part of the timePlot or the channel (?) , maybe part of the channel and just move across plots (?)
	auto timeplot = plotComponent->timePlot();
	auto xyplot = plotComponent->xyPlot();

	m_ch = ch;
	initPlotComponent(plotComponent);

	m_timePlotCh->xAxis()->setUnits("s");
	m_timePlotYAxis->setInterval(-2048,2048);
	m_xyPlotYAxis->setInterval(-2048,2048);

}

void PlotChannelComponent::deinitPlotComponent() {
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

void PlotChannelComponent::initPlotComponent(PlotComponent *plotComponent) {

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
//	m_timePlotAxisHandle->handle()->setHandlePos((HandlePos)yPlotAxisHandle);

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
}

PlotChannelComponent::~PlotChannelComponent() {

}

void PlotChannelComponent::disable() {
	ToolComponent::disable();
	m_timePlotCh->disable();
	if(m_timePlotAxisHandle) {
		m_timePlotAxisHandle->handle()->setVisible(false);
	}
}


void PlotChannelComponent::refreshData(bool copy) {

	auto data = m_ch->chData();
	if(copy) {
		m_timePlotCh->curve()->setSamples(data->xData(),data->yData(),data->size());
		if(m_xyXData) {
			m_xyPlotCh->curve()->setSamples(m_xyXData,data->yData(),data->size());
		}
	} else {
		m_timePlotCh->curve()->setRawSamples(data->xData(),data->yData(),data->size());
		if(m_xyXData) {
			m_xyPlotCh->curve()->setRawSamples(m_xyXData,data->yData(),data->size());
		}
	}

}

void PlotChannelComponent::onNewData(const float *xData_, const float *yData_, size_t size, bool copy) {
	refreshData(copy);
}

void PlotChannelComponent::setXyXData(const float *xyxdata) {
	m_xyXData = xyxdata;
}

QWidget *PlotChannelComponent::createCurveMenu(QWidget *parent) {

	MenuSectionWidget *curvecontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *curve = new MenuCollapseSection("CURVE", MenuCollapseSection::MHCW_NONE, curvecontainer);

	MenuPlotChannelCurveStyleControl *curveSettings = new MenuPlotChannelCurveStyleControl(curve);
	curveSettings->addChannels(m_timePlotCh);

	curve->contentLayout()->addWidget(curveSettings);
	curvecontainer->contentLayout()->addWidget(curve);

	return curvecontainer;
}

void PlotChannelComponent::enable() {
	ToolComponent::enable();
	m_timePlotCh->enable();
	if(m_timePlotAxisHandle) {
		m_timePlotAxisHandle->handle()->setVisible(true);
		m_timePlotAxisHandle->handle()->raise();
	}
}

PlotComponentManager::PlotComponentManager(QString name, QWidget *parent)
{
	m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);

	m_measurePanel = new MeasurementsPanel(this);
	m_measurePanel->setFixedHeight(100);
	//tool->topStack()->add(measureMenuId, m_measurePanel);

	m_statsPanel = new StatsPanel(this);
	m_statsPanel->setFixedHeight(80);
	//tool->bottomStack()->add(statsMenuId, m_statsPanel);
	m_lay->addWidget(m_measurePanel);
	m_measurePanel->setVisible(false);
	m_statsPanel->setVisible(false);
	m_lay->addWidget(m_statsPanel);

	addPlot("FirstPlot");
	addPlot("secondPlot");

}

PlotComponentManager::~PlotComponentManager()
{

}


void PlotComponentManager::enableMeasurementPanel(bool b)
{
	m_measurePanel->setVisible(b);
}

void PlotComponentManager::enableStatsPanel(bool b)
{
	m_statsPanel->setVisible(b);
}


MeasurementsPanel *PlotComponentManager::measurePanel() const
{
	return m_measurePanel;
}

StatsPanel *PlotComponentManager::statsPanel() const
{
	return m_statsPanel;
}

void PlotComponentManager::addPlot(QString name)
{
	PlotComponent *plt = new PlotComponent(name,this);
	m_plots.append(plt);
	addComponent(plt);
	m_lay->insertWidget(1, plt);
}

void PlotComponentManager::removePlot(QString name)
{	PlotComponent *plt = nullptr;
	for(PlotComponent *p : qAsConst(m_plots)) {
		if(p->name() == name) {
			plt = p;
		}
	}
	m_plots.removeAll(plt);
	removeComponent(plt);
	m_lay->removeWidget(plt);
}

void PlotComponentManager::addChannel(ChannelComponent *c, int idx)
{
	m_channels.append(c->plotChannelCmpt());
	m_plots[idx]->addChannel(c);
}


void PlotComponentManager::removeChannel(ChannelComponent *c)
{
	c->plotChannelCmpt()->m_plotComponent->removeChannel(c);
	m_channels.removeAll(c->plotChannelCmpt());
}

void PlotComponentManager::moveChannel(ChannelComponent *c, int newIdx) {
	c->plotChannelCmpt()->m_plotComponent->removeChannel(c);
	c->plotChannelCmpt()->initPlotComponent(m_plots[newIdx]);
	m_plots[newIdx]->addChannel(c);
}

PlotComponent *PlotComponentManager::plot(QString name)
{
	for(PlotComponent *p : m_plots) {
		if(p->name() == name) {
			return p;
		}
	}
	return nullptr;

}

PlotComponent *PlotComponentManager::plot(int i)
{
	PlotComponent *plt = m_plots[i];
	return plt;
}

QList<PlotComponent *> PlotComponentManager::plots() const
{
	return m_plots;
}

void PlotComponentManager::replot()
{
	for(PlotComponent *p : m_plots) {
		p->replot();
	}
}
