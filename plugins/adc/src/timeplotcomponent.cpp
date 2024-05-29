#include "timeplotcomponent.h"
#include "plotaxis.h"

#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include "widgets/menuplotchannelcurvestylecontrol.h"
#include "channelcomponent.h"
#include <pluginbase/preferences.h>

#include <qwt_point_data.h>
#include <QLineEdit>

using namespace scopy;
using namespace scopy::adc;
using namespace scopy::gui;

PlotComponent::PlotComponent(QString name, uint32_t uuid, QWidget *parent)
	: QWidget(parent)
	, m_uuid(uuid)
	, m_plotMenu(nullptr)
	, m_xAxisSrc(nullptr)
	, m_xAxisShow(nullptr)
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

ChannelComponent *PlotComponent::XYXChannel()
{
	return m_XYXChannel;
}

void PlotComponent::onStart() {}

void PlotComponent::onStop() {}

void PlotComponent::onInit() {

}

void PlotComponent::onDeinit() {}

void PlotComponent::setXYXChannel(ChannelComponent *c) {
	disconnect(xyDataConn);
	m_XYXChannel = c;
	if(c) {
		xyDataConn =  connect(c->chData(), &ChannelData::newData,
				     this, [=](const float* xData_, const float* yData_, size_t size, bool copy){
					     xyXData = yData_;
					     // just set xyxdata to plotchannel and let replot handle it ?
					     for(PlotChannelComponent *ch : qAsConst(m_channels)) {
						     ch->setXyXData(xyXData);
						     //ch->refreshData(copy);
					     }
				     });
	}
}

void PlotComponent::addChannel(ChannelComponent *c)
{
	m_channels.append(c->plotChannelCmpt());
	if(m_XYXChannel == nullptr) {
		setXYXChannel(c);
	}

	// https://stackoverflow.com/questions/44501171/qvariant-with-custom-class-pointer-does-not-return-same-address
	m_xAxisSrc->combo()->addItem(c->name(),QVariant::fromValue(static_cast<void*>(c)));
	m_autoscaler->addChannels(c->plotChannelCmpt()->m_timePlotCh);
}

void PlotComponent::removeChannel(ChannelComponent *c)
{
	PlotChannelComponent* toRemove;
	for(PlotChannelComponent* ch : qAsConst(m_channels)) {
		if(ch->m_ch == c) {
			toRemove = ch;
			break;
		}
	}
	m_channels.removeAll(toRemove);

	if(m_XYXChannel == c) {
		if(m_channels.size() > 0) {
			setXYXChannel(m_channels[0]->m_ch);
		}
		else {
			setXYXChannel(nullptr);
		}
	}

	int comboId = m_xAxisSrc->combo()->findData(QVariant::fromValue(static_cast<void*>(c)));
	m_xAxisSrc->combo()->removeItem(comboId);
	m_autoscaler->removeChannels(c->plotChannelCmpt()->m_timePlotCh);

}

bool PlotComponent::singleYMode() const
{
	return m_singleYMode;
}

QWidget *PlotComponent::createPlotMenu(QWidget *parent)
{
	// This could be refactored in it's own class
	MenuSectionWidget *w = new MenuSectionWidget(parent);
	MenuCollapseSection *plotMenu = new MenuCollapseSection("PLOT - " + m_name, MenuCollapseSection::MHCW_NONE, w);

	QLabel *plotTitleLabel = new QLabel("Plot title");
	StyleHelper::MenuSmallLabel(plotTitleLabel);
	QLineEdit *plotTitle = new QLineEdit(m_name);	
	StyleHelper::MenuLineEdit(plotTitle);
	connect(plotTitle, &QLineEdit::textChanged, this, [=](QString s){
		m_name = s;
		plotMenu->setTitle("PLOT - " + s);
	});

	MenuOnOffSwitch *labelsSwitch = new MenuOnOffSwitch("Show plot labels", plotMenu, false);
	connect(labelsSwitch->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b){
		m_timePlot->setShowXAxisLabels(b);
		m_timePlot->setShowYAxisLabels(b);

		m_xyPlot->setShowXAxisLabels(b);
		m_xyPlot->setShowYAxisLabels(b);

		m_timePlot->showAxisLabels();
		m_xyPlot->showAxisLabels();
	});



	// Need to set this for some reason .. spinboxes should be refactored
	m_timePlot->yAxis()->setUnits("V");
	MenuOnOffSwitch *singleYMode = new MenuOnOffSwitch("Single Y Mode", plotMenu, false);
	MenuPlotAxisRangeControl *m_yctrl = new MenuPlotAxisRangeControl(m_timePlot->yAxis(),this);

	QPushButton *m_autoscaleBtn = new QPushButton("Autoscale", plotMenu);
	StyleHelper::BlueButton(m_autoscaleBtn, "autoscale");
	m_autoscaler = new PlotAutoscaler(this);
	connect(m_autoscaler, &PlotAutoscaler::newMin, m_yctrl, &MenuPlotAxisRangeControl::setMin);
	connect(m_autoscaler, &PlotAutoscaler::newMax, m_yctrl, &MenuPlotAxisRangeControl::setMax);

	connect(m_autoscaleBtn,  &QPushButton::clicked, m_autoscaler, &PlotAutoscaler::autoscale);

	connect(singleYMode->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b){
		m_singleYMode = b;
		for(PlotChannelComponent *pcc : qAsConst(m_channels)) {
			pcc->setSingleYMode(b);
		}
		m_yctrl->setVisible(b);
		m_autoscaleBtn->setVisible(b);

	});

	MenuOnOffSwitch *xySwitch = new MenuOnOffSwitch("XY PLOT", plotMenu, false);

	m_xAxisSrc = new MenuCombo("XY - X Axis source");
	connect(m_xAxisSrc->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx){
		QComboBox *cb = m_xAxisSrc->combo();
		ChannelComponent *c = static_cast<ChannelComponent*>(cb->itemData(idx).value<void*>());
		if(m_XYXChannel) {
			m_XYXChannel->plotChannelCmpt()->m_xyPlotCh->setEnabled(true);
		}
		setXYXChannel(c);
		if(m_XYXChannel) {
			m_XYXChannel->plotChannelCmpt()->m_xyPlotCh->setEnabled(m_xAxisShow->onOffswitch()->isChecked());
		}
	});

	m_xAxisShow = new MenuOnOffSwitch("XY - Plot X source", plotMenu, false);

	connect(xySwitch->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_xyPlot->setVisible(b);
		m_xAxisSrc->setVisible(b);
		m_xAxisShow->setVisible(b);
	});

	connect(m_xAxisShow->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_XYXChannel->plotChannelCmpt()->m_xyPlotCh->setEnabled(b);
	});

	w->contentLayout()->addWidget(plotMenu);
	plotMenu->contentLayout()->addWidget(plotTitleLabel);
	plotMenu->contentLayout()->addWidget(plotTitle);

	plotMenu->contentLayout()->addWidget(singleYMode);
	plotMenu->contentLayout()->addWidget(m_yctrl);
	plotMenu->contentLayout()->addWidget(m_autoscaleBtn);

	plotMenu->contentLayout()->addWidget(labelsSwitch);
	plotMenu->contentLayout()->addWidget(xySwitch);
	plotMenu->contentLayout()->addWidget(m_xAxisSrc);
	plotMenu->contentLayout()->addWidget(m_xAxisShow);
	plotMenu->contentLayout()->setSpacing(10);


	m_plotMenu = w;

	// init
	xySwitch->onOffswitch()->setChecked(true);
	singleYMode->onOffswitch()->setChecked(true);
	return w;
}

void PlotComponent::deletePlotMenu() {
	delete m_plotMenu;
	m_plotMenu = nullptr;
}

QWidget *PlotComponent::plotMenu() {
	return m_plotMenu;
}

uint32_t PlotComponent::uuid()
{
	return m_uuid;
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

void PlotChannelComponent::setSingleYMode(bool b) {
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
	: m_plotIdx(0)
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

void PlotComponentManager::setXInterval(double xMin, double xMax)
{
	for(auto plt : qAsConst(m_plots)) {
		plt->timePlot()->xAxis()->setInterval(xMin, xMax);
	}
}

void PlotComponentManager::selectChannel(ChannelComponent *c)
{
	for(PlotChannelComponent *pcc : m_channels) {
		if(pcc->m_ch == c) {
			PlotChannel *ch = pcc->m_timePlotCh;
			PlotWidget* w = pcc->m_plotComponent->timePlot();
			w->selectChannel(ch);
		}
	}
}

MeasurementsPanel *PlotComponentManager::measurePanel() const
{
	return m_measurePanel;
}

StatsPanel *PlotComponentManager::statsPanel() const
{
	return m_statsPanel;
}

uint32_t PlotComponentManager::addPlot(QString name)
{
	PlotComponent *plt = new PlotComponent(name, m_plotIdx, this);
	m_plotIdx++;
	m_plots.append(plt);
	addComponent(plt);
	m_lay->insertWidget(1, plt);
	return plt->uuid();
}

void PlotComponentManager::removePlot(uint32_t uuid)
{	PlotComponent *plt = plot(uuid);
	m_plots.removeAll(plt);
	removeComponent(plt);
	m_lay->removeWidget(plt);
}

void PlotComponentManager::addChannel(ChannelComponent *c, uint32_t uuid)
{
	m_channels.append(c->plotChannelCmpt());
	PlotComponent *plt = plot(uuid);
	plt->addChannel(c);	
}


void PlotComponentManager::removeChannel(ChannelComponent *c)
{
	c->plotChannelCmpt()->m_plotComponent->removeChannel(c);
	m_channels.removeAll(c->plotChannelCmpt());
}

void PlotComponentManager::moveChannel(ChannelComponent *c, uint32_t uuid) {
	c->plotChannelCmpt()->m_plotComponent->removeChannel(c);
	PlotComponent *plt = plot(uuid);
	c->plotChannelCmpt()->initPlotComponent(plt);
	plt->addChannel(c);
}

PlotComponent *PlotComponentManager::plot(uint32_t uuid)
{
	PlotComponent *plt = nullptr;
	for(PlotComponent *p : qAsConst(m_plots)) {
		if(p->uuid() == uuid) {
			plt = p;
		}
	}
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
