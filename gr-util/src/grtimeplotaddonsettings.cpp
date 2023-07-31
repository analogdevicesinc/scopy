#include "grtimeplotaddonsettings.h"
#include "plotaxis.h"
#include "plotwidget.h"
#include <gui/widgets/menuheader.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menulineedit.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menucombo.h>
#include <QVBoxLayout>

using namespace scopy::grutil;

GRTimePlotAddonSettings::GRTimePlotAddonSettings(GRTimePlotAddon *p, QObject *parent) :
	QObject(parent),m_plot(p) {
	name = p->getName()+"_settings";
	m_sampleRateAvailable = false;
	widget = createMenu();
}

GRTimePlotAddonSettings::~GRTimePlotAddonSettings() {}

QWidget* GRTimePlotAddonSettings::createMenu(QWidget* parent) {
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setMargin(0);
	lay->setSpacing(10);
	w->setLayout(lay);

	m_pen = QPen(StyleHelper::getColor("ScopyBlue"));

	MenuHeaderWidget *header = new MenuHeaderWidget("PLOT", m_pen, w);
	QWidget* xaxismenu = createXAxisMenu(w);
//	QWidget* curvemenu = createCurveMenu(w);

	lay->addWidget(header);
	lay->addWidget(xaxismenu);
//	lay->addWidget(curvemenu);

	lay->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding));


	return w;
}


QWidget* GRTimePlotAddonSettings::createXAxisMenu(QWidget* parent) {
	MenuSectionWidget *xaxiscontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *xaxis = new MenuCollapseSection("X-AXIS",MenuCollapseSection::MHCW_NONE, xaxiscontainer);

	QWidget *bufferPlotSize = new QWidget(xaxis);
	QHBoxLayout *bufferPlotSizeLayout = new QHBoxLayout(bufferPlotSize);
	bufferPlotSizeLayout->setMargin(0);
	bufferPlotSizeLayout->setSpacing(10);
	bufferPlotSize->setLayout(bufferPlotSizeLayout);

	m_bufferSizeSpin = new ScaleSpinButton(
		{
		 {"samples",1e0},
		 {"ksamples",1e3},
		 {"Msamples",1e6},
		 },"Buffer Size",16,(double)((long)1<<31),false,false,bufferPlotSize);

	connect(m_bufferSizeSpin, &ScaleSpinButton::valueChanged, this, [=](double val){ setBufferSize((uint32_t)val);});
	connect(this, &GRTimePlotAddonSettings::bufferSizeChanged, m_plot, &GRTimePlotAddon::setBufferSize);

	m_plotSizeSpin = new ScaleSpinButton(
		{
		 {"samples",1e0},
		 {"ksamples",1e3},
		 {"Msamples",1e6},
		 {"Gsamples",1e9},
		 },"Plot Size",16,(double)((long)1<<31),false,false,bufferPlotSize);

	connect(m_plotSizeSpin, &ScaleSpinButton::valueChanged, this, [=](double val){ setPlotSize((uint32_t)val);});
	connect(this, &GRTimePlotAddonSettings::plotSizeChanged, m_plot, &GRTimePlotAddon::setPlotSize);


	bufferPlotSizeLayout->addWidget(m_bufferSizeSpin);
	bufferPlotSizeLayout->addWidget(m_plotSizeSpin);

	MenuOnOffSwitch *syncBufferPlot = new MenuOnOffSwitch(tr("SYNC BUFFER-PLOT SIZES"), xaxis, false);
	m_rollingModeSw = new MenuOnOffSwitch(tr("ROLLING MODE"), xaxis, false);
	connect(m_rollingModeSw->onOffswitch(), &QAbstractButton::toggled, this, &GRTimePlotAddonSettings::setRollingMode);
	connect(this, &GRTimePlotAddonSettings::rollingModeChanged, m_plot, &GRTimePlotAddon::setRollingMode);

	QWidget *xMinMax = new QWidget(xaxis);
	QHBoxLayout *xMinMaxLayout = new QHBoxLayout(xMinMax);
	xMinMaxLayout->setMargin(0);
	xMinMaxLayout->setSpacing(10);
	xMinMax->setLayout(xMinMaxLayout);

	m_xmin = new PositionSpinButton(
		{
		 {"V",1e0},
		 {"k",1e3},
		 {"M",1e6},
		 {"G",1e9},
		 },"XMin",(double)((long)(-1<<31)),(double)((long)1<<31),false,false,xMinMax);

	m_xmax = new PositionSpinButton(
		{
		 {"V",1e0},
		 {"k",1e3},
		 {"M",1e6},
		 {"G",1e9},
		 },"XMax",(double)((long)(-1<<31)),(double)((long)1<<31),false,false,xMinMax);

	auto m_plotAxis = m_plot->plot()->xAxis();
	// Connects
	connect(m_xmin, &PositionSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMin);
	connect(m_plotAxis, &PlotAxis::minChanged, this, [=](){
		QSignalBlocker b(m_xmin);
		m_xmin->setValue(m_plotAxis->min());
	});

	connect(m_xmax, &PositionSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMax);
	connect(m_plotAxis, &PlotAxis::maxChanged, this, [=](){
		QSignalBlocker b(m_xmax);
		m_xmax->setValue(m_plotAxis->max());
	});

	xMinMaxLayout->addWidget(m_xmin);
	xMinMaxLayout->addWidget(m_xmax);

	MenuCombo *cbb = new MenuCombo("XMode", xaxis);
	auto cb = cbb->combo();

	cb->addItem("Samples", XMODE_SAMPLES);
	if(m_sampleRateAvailable) {
		cb->addItem("Time", XMODE_TIME);
	}
	cb->addItem("Time - override samplerate", XMODE_OVERRIDE);

	m_sampleRateSpin = new PositionSpinButton(
		{
		 {"Hz",1e0},
		 {"kHz",1e3},
		 {"MHz",1e6},
		 {"GHz",1e9},
		 },"SampleRate",1,(double)((long)1<<31),false,false,xaxis);

	xaxiscontainer->contentLayout()->setSpacing(10);
	xaxiscontainer->contentLayout()->addWidget(xaxis);
	xaxis->contentLayout()->addWidget(bufferPlotSize);
	xaxis->contentLayout()->addWidget(syncBufferPlot);
	xaxis->contentLayout()->addWidget(m_rollingModeSw);
	xaxis->contentLayout()->addWidget(xMinMax);
	xaxis->contentLayout()->addWidget(cbb);
	xaxis->contentLayout()->addWidget(m_sampleRateSpin);
	xaxis->contentLayout()->setSpacing(10);

	return xaxiscontainer;
}

bool GRTimePlotAddonSettings::rollingMode() const
{
	return m_rollingMode;
}

void GRTimePlotAddonSettings::setRollingMode(bool newRollingMode)
{
	if (m_rollingMode == newRollingMode)
		return;
	m_rollingMode = newRollingMode;
	Q_EMIT rollingModeChanged(newRollingMode);
}

uint32_t GRTimePlotAddonSettings::plotSize() const
{
	return m_plotSize;
}

void GRTimePlotAddonSettings::setPlotSize(uint32_t newPlotSize)
{
	if (m_plotSize == newPlotSize)
		return;
	m_plotSize = newPlotSize;
	Q_EMIT plotSizeChanged(newPlotSize);
}

void GRTimePlotAddonSettings::onInit() {
	m_bufferSizeSpin->setValue(32);
	m_plotSizeSpin->setValue(32);
	m_xmin->setValue(0);
	m_xmax->setValue(31);
	m_rollingModeSw->onOffswitch()->setChecked(false);
}

void GRTimePlotAddonSettings::onDeinit() {

}

uint32_t GRTimePlotAddonSettings::bufferSize() const
{
	return m_bufferSize;
}

void GRTimePlotAddonSettings::setBufferSize(uint32_t newBufferSize)
{
	if (m_bufferSize == newBufferSize)
		return;
	m_bufferSize = newBufferSize;
	Q_EMIT bufferSizeChanged(newBufferSize);
}

QString GRTimePlotAddonSettings::getName() { return name;}

QWidget *GRTimePlotAddonSettings::getWidget() { return widget;}
