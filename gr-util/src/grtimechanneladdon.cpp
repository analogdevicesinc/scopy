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

Q_LOGGING_CATEGORY(CAT_GR_TIME_CHANNEL, "GRTimeChannel");
using namespace scopy::grutil;

QWidget* GRTimeChannelAddon::createYAxisMenu(QWidget* parent) {
	MenuSectionWidget *yaxiscontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *yaxis = new MenuCollapseSection("Y-AXIS",MenuCollapseSection::MHCW_NONE, yaxiscontainer);
	MenuCombo *cbb = new MenuCombo("", yaxis);
	auto cb = cbb->combo();
	cb->addItem("ADC Counts", YMODE_COUNT);
	cb->addItem("% Full Scale", YMODE_FS);

	if(m_scaleAvailable) {
		cb->addItem(m_unit, YMODE_SCALE);
	}

	QWidget *startStop = new QWidget(yaxis);
	QHBoxLayout *startStopLay = new QHBoxLayout(startStop);
	startStopLay->setMargin(0);
	startStopLay->setSpacing(10);
	startStop->setLayout(startStopLay);

	m_ymin = new ScaleSpinButton(
		{
		 {"V",1e0},
		 {"k",1e3},
		 {"M",1e6},
		 {"G",1e9},
		 },"YMin",(double)((long)(-1<<31)),(double)((long)1<<31),false,false,startStop);


	m_ymax = new ScaleSpinButton(
	    {
	     {"V",1e0},
	     {"k",1e3},
	     {"M",1e6},
	     {"G",1e9},
	     },"YMax",(double)((long)(-1<<31)),(double)((long)1<<31),false,false,startStop);

	startStopLay->addWidget(m_ymin);
	startStopLay->addWidget(m_ymax);

	m_autoScaleTimer = new QTimer(this);
	m_autoScaleTimer->setInterval(1000);
	connect(m_autoScaleTimer, &QTimer::timeout, this, &GRTimeChannelAddon::autoscale);
	MenuOnOffSwitch *autoscale = new MenuOnOffSwitch(tr("AUTOSCALE"), yaxis, false);

	yaxis->add(startStop);
	yaxis->add(autoscale);
	yaxis->add(cbb);

	yaxiscontainer->contentLayout()->addWidget(yaxis);

	// Connects
	connect(m_ymin, &ScaleSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMin);
	connect(m_plotAxis, &PlotAxis::minChanged, this, [=](){
		QSignalBlocker b(m_ymin);
		m_ymin->setValue(m_plotAxis->min());
	});

	connect(m_ymax, &ScaleSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMax);
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

	// Defaults
	m_ymin->setValue(-1.0);
	m_ymax->setValue(1.0);
	cb->setCurrentIndex(1);

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
	curve->add(curveSettings);
	curvecontainer->contentLayout()->addWidget(curve);

	return curvecontainer;
}

QWidget* GRTimeChannelAddon::createMenu(QWidget* parent) {
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setMargin(0);
	lay->setSpacing(10);
	w->setLayout(lay);

	MenuHeaderWidget *header = new MenuHeaderWidget(m_channelName, m_pen, w);
	QWidget* yaxismenu = createYAxisMenu(w);
	QWidget* curvemenu = createCurveMenu(w);

	lay->addWidget(header);
	lay->addWidget(yaxismenu);
	lay->addWidget(curvemenu);

	lay->addSpacerItem(new QSpacerItem(40,40,QSizePolicy::Minimum,QSizePolicy::Expanding));


	return w;
}

GRTimeChannelAddon::GRTimeChannelAddon(QString ch, GRDeviceAddon *dev, GRTimePlotAddon *plotAddon, QPen pen, QObject *parent)
    : QObject(parent), m_channelName(ch), m_dev(dev), m_plotAddon(plotAddon), m_pen(pen) {

	m_running = false;
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

	m_plotAxis = new PlotAxis(QwtAxis::YLeft, plot, pen, this);
	m_plotCh = new PlotChannel(name, pen, plot, plot->xAxis(), m_plotAxis,this);
	m_plotAxisHandle = new PlotAxisHandle(pen, m_plotAxis,plot,this);
	m_plotCh->setHandle(m_plotAxisHandle);
	plot->addPlotAxisHandle(m_plotAxisHandle);

	widget = createMenu();

	// - create ADCCount/VFS/V
}

GRTimeChannelAddon::~GRTimeChannelAddon() {}

QString GRTimeChannelAddon::getName() {return name;}

QWidget *GRTimeChannelAddon::getWidget() {return widget;}

void GRTimeChannelAddon::setDevice(GRDeviceAddon *d) { m_dev = d; d->registerChannel(this);}

GRDeviceAddon* GRTimeChannelAddon::getDevice() { return m_dev;}

void GRTimeChannelAddon::enable() {
	qInfo(CAT_GR_TIME_CHANNEL)<<name<<" enabled";
	m_plotCh->enable();
	m_plotAxisHandle->handle()->setVisible(true);
	m_plotAxisHandle->handle()->raise();
	m_signalPath->setEnabled(true);
	m_grch->setEnabled(true);
	m_plotAddon->plot()->replot();
}


void GRTimeChannelAddon::disable() {
	qInfo(CAT_GR_TIME_CHANNEL())<<name<<" disabled";
	m_plotCh->disable();
	m_plotAxisHandle->handle()->setVisible(false);
	m_signalPath->setEnabled(false);
	m_grch->setEnabled(false);
	m_plotAddon->plot()->replot();
}

void GRTimeChannelAddon::onStart() {
	m_running = true;
	toggleAutoScale();
}

void GRTimeChannelAddon::onStop() {
	m_running = false;
	toggleAutoScale();
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


void GRTimeChannelAddon::onAdd() {  }

void GRTimeChannelAddon::onRemove() {}

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
