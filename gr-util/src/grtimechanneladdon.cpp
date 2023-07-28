#include "grtimechanneladdon.h"
#include "grdeviceaddon.h"
#include "griiofloatchannelsrc.h"
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

using namespace scopy::grutil;

QWidget* GRTimeChannelAddon::createYAxisMenu(QWidget* parent) {
	MenuSectionWidget *yaxiscontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *yaxis = new MenuCollapseSection("Y-AXIS",MenuCollapseSection::MHCW_NONE, yaxiscontainer);

	MenuCombo *cbb = new MenuCombo("", yaxis);
	auto cb = cbb->combo();
	cb->addItem("ADC Counts", 1);
	cb->addItem("% Full Scale", 1.0/(1<<12));
	if(m_scaleAvailable) {
		cb->addItem(m_unit, 2.2);
	}

	connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		m_scOff->setScale(cb->itemData(idx).toDouble());

	});

	QWidget *startStop = new QWidget(yaxis);
	QHBoxLayout *startStopLay = new QHBoxLayout(startStop);
	startStopLay->setMargin(0);
	startStopLay->setSpacing(10);
	startStop->setLayout(startStopLay);

	ScaleSpinButton *ymin = new ScaleSpinButton(
		{
		 {" ",1e0},
		 {"k",1e3},
		 {"M",1e6},
		 {"G",1e9},
		 },"YMin",(double)((long)(-1<<31)),(double)((long)1<<31),false,false,startStop);
//	connect(ymin, &ScaleSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMin);
//	connect(m_plotAxis, &PlotAxis::minChanged, this, [=](){
////		QSignalBlocker b(ymin);
//		ymin->setValue(m_plotAxis->min());
//	});

	ScaleSpinButton *ymax = new ScaleSpinButton(
	    {
	     {" ",1e0},
	     {"k",1e3},
	     {"M",1e6},
	     {"G",1e9},
	     },"YMax",(double)((long)(-1<<31)),(double)((long)1<<31),false,false,startStop);

//	connect(ymax, &ScaleSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMax);
//	connect(m_plotAxis, &PlotAxis::maxChanged, this, [=](){
////		QSignalBlocker b(ymax);
//		ymax->setValue(m_plotAxis->min());
//	});

	connect(ymin, &ScaleSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMin);

	ymin->setValue(-1.0);
	ymax->setValue(1.0);

	startStopLay->addWidget(ymin);
	startStopLay->addWidget(ymax);

	MenuOnOffSwitch *autoscale = new MenuOnOffSwitch(tr("AUTOSCALE"), yaxis, false);
	connect(autoscale->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b){
		ymin->setEnabled(!b);
		ymax->setEnabled(!b);
		m_plotAddon->plot()->plot()->setAxisAutoScale(m_plotAxis->axisId(), b);
	} );

	yaxis->add(startStop);
	yaxis->add(autoscale);
	yaxis->add(cbb);

	yaxiscontainer->contentLayout()->addWidget(yaxis);

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
	m_plotAxis->setInterval(-1<<11,1<<11);
//	m_plotAxis->setInterval(0,1<<24);
	m_plotCh = new PlotChannel(name, pen, plot, plot->xAxis(), m_plotAxis,this);
	m_plotAxisHandle = new PlotAxisHandle(pen, m_plotAxis,plot,this);
	m_plotCh->setHandle(m_plotAxisHandle);
	plot->addPlotAxisHandle(m_plotAxisHandle);

	// create menu widget
	widget = createMenu();

	// - create ADCCount/VFS/V
	// - create VDIV scale

	// - create thickness/render mode / Color (?)  / match to Curve
}

GRTimeChannelAddon::~GRTimeChannelAddon() {}

QString GRTimeChannelAddon::getName() {return name;}

QWidget *GRTimeChannelAddon::getWidget() {return widget;}

void GRTimeChannelAddon::setDevice(GRDeviceAddon *d) { m_dev = d; d->registerChannel(this);}

GRDeviceAddon* GRTimeChannelAddon::getDevice() { return m_dev;}

void GRTimeChannelAddon::enable() {
	qInfo()<<name<<" enabled";
	m_plotCh->attach();
	m_plotAxisHandle->handle()->setVisible(true);
	m_plotAxisHandle->handle()->raise();
	m_signalPath->setEnabled(true);
	m_grch->setEnabled(true);
}


void GRTimeChannelAddon::disable() {
	qInfo()<<name<<" disabled";
	m_plotCh->detach();
	m_plotAxisHandle->handle()->setVisible(false);
	m_signalPath->setEnabled(false);
	m_grch->setEnabled(false);
}

void GRTimeChannelAddon::onStart() {}

void GRTimeChannelAddon::onStop() {}

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
