#include "grtimechanneladdon.h"
#include "grdeviceaddon.h"
#include <gui/stylehelper.h>
#include <QDebug>
#include <QVBoxLayout>
#include <QComboBox>

#include <gui/widgets/menuheader.h>
#include <gui/stylehelper.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menubigswitch.h>
#include <gui/widgets/menulineedit.h>
#include <QLoggingCategory>
#include <pluginbase/preferences.h>
#include "measurementselector.h"

Q_LOGGING_CATEGORY(CAT_GR_TIME_CHANNEL, "GRTimeChannel");
using namespace scopy::grutil;

GRTimeChannelAddon::GRTimeChannelAddon(QString ch, GRDeviceAddon *dev, GRTimePlotAddon *plotAddon, QPen pen, QObject *parent)
    : QObject(parent), m_channelName(ch), m_dev(dev), m_plotAddon(plotAddon), m_pen(pen) {

	int yPlotAxisPosition = Preferences::get("adc_plot_yaxis_label_position").toInt();
	int yPlotAxisHandle = Preferences::get("adc_plot_yaxis_handle_position").toInt();

	m_running = false;
	m_enabled = false;
	m_autoscaleEnabled = false;
	m_signalPath = new GRSignalPath("time_" + dev->getName() + ch, this);
	m_grch = new GRIIOFloatChannelSrc(dev->src(),ch,m_signalPath);
	m_signalPath->append(m_grch);
	m_scOff = new GRScaleOffsetProc(m_signalPath);
	m_signalPath->append(m_scOff);
	m_scOff->setOffset(0);
	m_scOff->setScale(1);
	m_signalPath->setEnabled(false);

	m_scaleAvailable = true; // query from GRIIOFloatChannel;
	m_unit = "Volts"; // query from GRIIOFloatChannel;

	setDevice(dev);

	name = m_grch->getChannelName();
	auto plot = plotAddon->plot();;

	m_plotAxis = new PlotAxis(yPlotAxisPosition, plot, pen, this);
	m_plotCh = new PlotChannel(name, pen, plot, plot->xAxis(), m_plotAxis,this);
	m_plotAxisHandle = new PlotAxisHandle(pen, m_plotAxis, plot, yPlotAxisHandle,this);
	m_plotCh->setHandle(m_plotAxisHandle);
	plot->addPlotAxisHandle(m_plotAxisHandle);
	plot->addPlotChannel(m_plotCh);

	initMeasure();
	widget = createMenu();
	m_sampleRateAvailable = m_grch->sampleRateAvailable();

}

GRTimeChannelAddon::~GRTimeChannelAddon() {}

QWidget* GRTimeChannelAddon::createYAxisMenu(QWidget* parent) {
	MenuSectionWidget *yaxiscontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *yaxis = new MenuCollapseSection("Y-AXIS",MenuCollapseSection::MHCW_NONE, yaxiscontainer);

	// Y-MODE
	m_ymodeCb = new MenuCombo("YMODE", yaxis);
	auto cb = m_ymodeCb->combo();
	cb->addItem("ADC Counts", YMODE_COUNT);
	cb->addItem("% Full Scale", YMODE_FS);

	if(m_scaleAvailable) {
		cb->addItem(m_unit, YMODE_SCALE);
	}

	// Y-MIN-MAX
	QWidget *yMinMax = new QWidget(yaxis);
	QHBoxLayout *yMinMaxLayout = new QHBoxLayout(yMinMax);
	yMinMaxLayout->setMargin(0);
	yMinMaxLayout->setSpacing(10);
	yMinMax->setLayout(yMinMaxLayout);

	m_ymin = new PositionSpinButton(
		{
		 {"V",1e0},
		 {"k",1e3},
		 {"M",1e6},
		 {"G",1e9},
		 },"YMin",(double)((long)(-1<<31)),(double)((long)1<<31),false,false,yMinMax);


	m_ymax = new PositionSpinButton(
	    {
	     {"V",1e0},
	     {"k",1e3},
	     {"M",1e6},
	     {"G",1e9},
	     },"YMax",(double)((long)(-1<<31)),(double)((long)1<<31),false,false,yMinMax);

	yMinMaxLayout->addWidget(m_ymin);
	yMinMaxLayout->addWidget(m_ymax);

	// AUTOSCALE
	m_autoScaleTimer = new QTimer(this);
	m_autoScaleTimer->setInterval(1000);
	connect(m_autoScaleTimer, &QTimer::timeout, this, &GRTimeChannelAddon::autoscale);
	MenuOnOffSwitch *autoscale = new MenuOnOffSwitch(tr("AUTOSCALE"), yaxis, false);

	yaxis->contentLayout()->addWidget(autoscale);
	yaxis->contentLayout()->addWidget(yMinMax);
	yaxis->contentLayout()->addWidget(m_ymodeCb);

	yaxiscontainer->contentLayout()->addWidget(yaxis);

	// Connects
	connect(m_ymin, &PositionSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMin);
	connect(m_plotAxis, &PlotAxis::minChanged, this, [=](){
		QSignalBlocker b(m_ymin);
		m_ymin->setValue(m_plotAxis->min());
	});

	connect(m_ymax, &PositionSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMax);
	connect(m_plotAxis, &PlotAxis::maxChanged, this, [=](){
		QSignalBlocker b(m_ymax);
		m_ymax->setValue(m_plotAxis->max());
	});

	connect(autoscale->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b){
		m_ymin->setEnabled(!b);
		m_ymax->setEnabled(!b);
		m_autoscaleEnabled = b;
		toggleAutoScale();

	} );

	connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		auto mode = cb->itemData(idx).toInt();
		setYMode(static_cast<YMode>(mode));
	});


	return yaxiscontainer;
}

QWidget* GRTimeChannelAddon::createCurveMenu(QWidget* parent) {

	MenuSectionWidget *curvecontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *curve = new MenuCollapseSection("CURVE",MenuCollapseSection::MHCW_NONE, curvecontainer);

	QWidget *curveSettings = new QWidget(curve);
	QHBoxLayout *curveSettingsLay = new QHBoxLayout(curveSettings);
	curveSettingsLay->setMargin(0);
	curveSettingsLay->setSpacing(10);
	curveSettings->setLayout(curveSettingsLay);

	MenuCombo *cbThicknessW = new MenuCombo("Thickness", curve);
	auto cbThickness = cbThicknessW->combo();
	cbThickness->addItem("1");
	cbThickness->addItem("2");
	cbThickness->addItem("3");
	cbThickness->addItem("4");
	cbThickness->addItem("5");

	connect(cbThickness, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		m_plotCh->setThickness(cbThickness->itemText(idx).toFloat());
	});
	MenuCombo *cbStyleW = new MenuCombo("Style", curve);
	auto cbStyle = cbStyleW->combo();
	cbStyle->addItem("Lines", PlotChannel::PCS_LINES);
	cbStyle->addItem("Dots", PlotChannel::PCS_DOTS);
	cbStyle->addItem("Steps", PlotChannel::PCS_STEPS);
	cbStyle->addItem("Sticks", PlotChannel::PCS_STICKS);
	cbStyle->addItem("Smooth", PlotChannel::PCS_SMOOTH);
	StyleHelper::MenuComboBox(cbStyle,"cbStyle");

	connect(cbStyle, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		m_plotCh->setStyle(cbStyle->itemData(idx).toInt());
	});

	curveSettingsLay->addWidget(cbThicknessW);
	curveSettingsLay->addWidget(cbStyleW);
	curve->contentLayout()->addWidget(curveSettings);
	curvecontainer->contentLayout()->addWidget(curve);

	return curvecontainer;
}

void GRTimeChannelAddon::initMeasure() {
	m_measureModel = new TimeMeasureModel(nullptr,0,this);
	m_measureController = new TimeChannelMeasurementController(m_measureModel, m_pen, this);
	m_measureModel->setAdcBitCount(grch()->getFmt()->bits);

	connect(m_measureController, &TimeChannelMeasurementController::measurementEnabled, this, &GRTimeChannelAddon::enableMeasurement);
	connect(m_measureController, &TimeChannelMeasurementController::measurementDisabled, this, &GRTimeChannelAddon::disableMeasurement);
	connect(m_measureController, &TimeChannelMeasurementController::statsEnabled, this, &GRTimeChannelAddon::enableStat);
	connect(m_measureController, &TimeChannelMeasurementController::statsDisabled, this, &GRTimeChannelAddon::disableStat);

}

QWidget* GRTimeChannelAddon::createMeasurementMenuSection(QString category, QWidget* parent) {


	MenuSectionWidget *measureContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *measureSection = new MenuCollapseSection("MEASUREMENT " + category,MenuCollapseSection::MHCW_ARROW, measureContainer);
	QScrollArea *measureScroll = new QScrollArea(measureSection);
	MeasurementSelector *measureSelector = new MeasurementSelector();
	measureContainer->contentLayout()->addWidget(measureSection);
	measureSection->contentLayout()->addWidget(measureScroll);
	measureScroll->setWidget(measureSelector);
	measureScroll->setWidgetResizable(true);

	measureScroll->setFixedHeight(150);

	for(auto &meas : m_measureController->availableMeasurements()) {
		if(meas.type.toUpper() == category.toUpper()) {
			measureSelector->addMeasurement(meas.name, meas.icon);
			connect(measureSelector->measurement(meas.name)->measureCheckbox(), &QCheckBox::toggled, [=](bool b){
				if(b)
					m_measureController->enableMeasurement(meas.name);
				else
					m_measureController->disableMeasurement(meas.name);
			});

			connect(measureSelector->measurement(meas.name)->statsCheckbox(), &QCheckBox::toggled, [=](bool b){
				if(b)
					m_measureController->enableStats(meas.name);
				else
					m_measureController->disableStats(meas.name);
			});
		}
	}
	measureSection->header()->setChecked(false);

	return measureContainer;
}

QWidget* GRTimeChannelAddon::createMenu(QWidget* parent) {
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout(w);

	QScrollArea *scroll = new QScrollArea(parent);
	QWidget *wScroll = new QWidget(scroll);
	QVBoxLayout *layScroll = new QVBoxLayout(wScroll);
	layScroll->setMargin(0);
	layScroll->setSpacing(10);

	wScroll->setLayout(layScroll);
	scroll->setWidgetResizable(true);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//if ScrollBarAlwaysOn - layScroll->setContentsMargins(0,0,6,0);

	scroll->setWidget(wScroll);

	lay->setMargin(0);
	lay->setSpacing(10);
	w->setLayout(lay);

	MenuHeaderWidget *header = new MenuHeaderWidget(m_channelName, m_pen, w);
	QWidget* yaxismenu = createYAxisMenu(w);
	QWidget* curvemenu = createCurveMenu(w);

	QWidget *hMeasure = createMeasurementMenuSection("HORIZONTAL", parent);
	QWidget *vMeasure = createMeasurementMenuSection("VERTICAL", parent);

	lay->addWidget(header);
	lay->addWidget(scroll);
	layScroll->addWidget(yaxismenu);
	layScroll->addWidget(curvemenu);
	layScroll->addWidget(hMeasure);
	layScroll->addWidget(vMeasure);

	layScroll->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding));


	return w;
}

QString GRTimeChannelAddon::getName() {return name;}

QWidget *GRTimeChannelAddon::getWidget() {return widget;}

void GRTimeChannelAddon::setDevice(GRDeviceAddon *d) { m_dev = d; d->registerChannel(this);}

GRDeviceAddon* GRTimeChannelAddon::getDevice() { return m_dev;}

void GRTimeChannelAddon::enable() {
	qInfo(CAT_GR_TIME_CHANNEL)<<name<<" enabled";
	m_enabled = true;
	m_plotCh->enable();
	m_plotAxisHandle->handle()->setVisible(true);
	m_plotAxisHandle->handle()->raise();
	m_signalPath->setEnabled(true);
//	m_grch->setEnabled(true);
	m_plotAddon->replot();
//	m_plotAddon->plot()->replot();
}


void GRTimeChannelAddon::disable() {
	qInfo(CAT_GR_TIME_CHANNEL())<<name<<" disabled";
	m_enabled = false;
	m_plotCh->disable();
	m_plotAxisHandle->handle()->setVisible(false);
	m_signalPath->setEnabled(false);
//	m_grch->setEnabled(false);
	m_plotAddon->replot();
//	m_plotAddon->plot()->replot();
}

void GRTimeChannelAddon::onStart() {
	m_running = true;
	m_measureModel->setSampleRate(m_plotAddon->sampleRate());
	toggleAutoScale();
}

void GRTimeChannelAddon::onStop() {	
	m_running = false;
	toggleAutoScale();
	if(m_autoscaleEnabled) {
		autoscale();
	}
}

void GRTimeChannelAddon::toggleAutoScale() {
	if(m_running && m_autoscaleEnabled) {
		m_autoScaleTimer->start();
	} else {
		m_autoScaleTimer->stop();
	}
}

void GRTimeChannelAddon::autoscale() {
	double max = -1000000.0;
	double min = 1000000.0;

	auto data =  m_plotCh->curve()->data();
	for(int i = 0; i < data->size(); i++) {
		auto sample = data->sample(i).y();
		if(max < sample) max = sample;
		if(min > sample) min = sample;
	}

	qInfo(CAT_GR_TIME_CHANNEL)<<"Autoscaling channel " << m_channelName << "to (" <<min <<", " << max<<")";

	m_plotAxis->setMin(min);
	m_plotAxis->setMax(max);
}

void GRTimeChannelAddon::setYMode(YMode mode)
{
	double scale = 1;
	double offset = 0;
	double ymin = -1;
	double ymax = 1;
	const iio_data_format* fmt = m_grch->getFmt();

	switch(mode) {
	case YMODE_COUNT:
		scale = 1;
		if(fmt->is_signed) {
			ymin = - 1<<(fmt->bits-1);
			ymax =   1<<(fmt->bits-1);
		} else {
			ymin =   0;
			ymax =   1<<(fmt->bits);
		}
		break;
	case YMODE_FS:
		scale = 1.0/((1<<fmt->bits));
		if(fmt->is_signed) {
			ymin = -0.5;
			ymax = 0.5;
		} else {
			ymin = 0;
			ymax = 1;
		}
		break;
	case YMODE_SCALE:
		break;
	default:
		break;

	}
	m_ymin->setValue(ymin);
	m_ymax->setValue(ymax);
	m_scOff->setScale(scale);
	m_scOff->setOffset(offset);

}

GRIIOFloatChannelSrc *GRTimeChannelAddon::grch() const
{
	return m_grch;
}

bool GRTimeChannelAddon::sampleRateAvailable() const
{
	return m_sampleRateAvailable;
}


void GRTimeChannelAddon::onInit() {
	// Defaults
	m_ymin->setValue(-1.0);
	m_ymax->setValue(1.0);
	m_ymodeCb->combo()->setCurrentIndex(1);
}

void GRTimeChannelAddon::onDeinit() {}

void GRTimeChannelAddon::preFlowBuild()
{
	double m_sampleRate = m_grch->readSampleRate();
	qInfo()<<"READ SAMPLE RATE" << m_sampleRate;
}

void GRTimeChannelAddon::onNewData(const float* xData, const float* yData, int size)
{
	m_measureModel->setDataSource(yData, size);
	m_measureModel->measure();
}

void GRTimeChannelAddon::onChannelAdded(ToolAddon *) {}

void GRTimeChannelAddon::onChannelRemoved(ToolAddon *) {}

PlotChannel *GRTimeChannelAddon::plotCh() const
{
	return m_plotCh;
}

GRSignalPath *GRTimeChannelAddon::signalPath() const
{
	return m_signalPath;
}

QPen GRTimeChannelAddon::pen() const {
	return m_pen;
}

bool GRTimeChannelAddon::enabled() const
{
	return m_enabled;
}
