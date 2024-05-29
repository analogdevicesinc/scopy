#include "timeplotmanagersettings.h"
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuheader.h>
#include <gui/stylehelper.h>
#include <timeplotcomponentsettings.h>

namespace scopy {
namespace adc {

TimePlotManagerSettings::TimePlotManagerSettings(TimePlotManager *mgr, QWidget *parent)
	: QWidget(parent)
	, ToolComponent()
	, m_syncMode(false)
	, m_sampleRateAvailable(false)
{
	m_plotManager = mgr;
	auto *w = createMenu(this);
	QVBoxLayout *lay = new QVBoxLayout(parent);
	lay->addWidget(w);
	lay->setSpacing(0);
	lay->setMargin(0);
	setLayout(lay);
}

TimePlotManagerSettings::~TimePlotManagerSettings() {}

QWidget *TimePlotManagerSettings::createMenu(QWidget *parent)
{
	QScrollArea *scroll = new QScrollArea(parent);
	scroll->setWidgetResizable(true);
	QWidget *w = new QWidget(scroll);
	scroll->setWidget(w);
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setMargin(0);
	lay->setSpacing(10);
	w->setLayout(lay);

	m_pen = QPen(StyleHelper::getColor("ScopyBlue"));

	MenuHeaderWidget *header = new MenuHeaderWidget("TIME PLOT", m_pen, w);
	QWidget *xaxismenu = createXAxisMenu(w);

	m_plotContainerLayout = new QVBoxLayout(w);
	m_plotContainerLayout->setMargin(0);
	m_plotContainerLayout->setSpacing(10);

	m_addPlotBtn = new QPushButton("Add Plot", this);
	StyleHelper::BlueButton(m_addPlotBtn, "AddPlotButton");

	connect(m_addPlotBtn, &QPushButton::clicked, this, [=](){
		uint32_t idx = m_plotManager->addPlot("Plot ");
		TimePlotComponent *plt = m_plotManager->plot(idx);
		addPlot(plt);
	});

	lay->addWidget(header);
	lay->addWidget(xaxismenu);
	lay->addLayout(m_plotContainerLayout);
	lay->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding));
	lay->addWidget(m_addPlotBtn);


	return scroll;
}

QWidget *TimePlotManagerSettings::createXAxisMenu(QWidget *parent)
{
	MenuSectionWidget *xaxiscontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *xaxis = new MenuCollapseSection("X-AXIS", MenuCollapseSection::MHCW_NONE, xaxiscontainer);

	QWidget *bufferPlotSize = new QWidget(xaxis);
	QHBoxLayout *bufferPlotSizeLayout = new QHBoxLayout(bufferPlotSize);
	bufferPlotSizeLayout->setMargin(0);
	bufferPlotSizeLayout->setSpacing(10);
	bufferPlotSize->setLayout(bufferPlotSizeLayout);

	m_bufferSizeSpin = new ScaleSpinButton(
		{
		 {"samples", 1e0},
		 {"ksamples", 1e3},
		 {"Msamples", 1e6},
		 },
		"Buffer Size", 16, DBL_MAX, false, false, bufferPlotSize);

	connect(m_bufferSizeSpin, &ScaleSpinButton::valueChanged, this, [=](double val) {
		if(m_plotSizeSpin->value() < val) {
			m_plotSizeSpin->setValue(val);
		}
		m_plotSizeSpin->setMinValue(val);
		setBufferSize((uint32_t)val);
	});

	m_plotSizeSpin = new ScaleSpinButton(
		{
		 {"samples", 1e0},
		 {"ksamples", 1e3},
		 {"Msamples", 1e6},
		 {"Gsamples", 1e9},
		 },
		"Plot Size", 16, DBL_MAX, false, false, bufferPlotSize);

	connect(m_plotSizeSpin, &ScaleSpinButton::valueChanged, this, [=](double val) { setPlotSize((uint32_t)val); });

	bufferPlotSizeLayout->addWidget(m_bufferSizeSpin);
	bufferPlotSizeLayout->addWidget(m_plotSizeSpin);

	m_syncBufferPlot = new MenuOnOffSwitch(tr("SYNC BUFFER-PLOT SIZES"), xaxis, false);
	connect(m_syncBufferPlot->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_plotSizeSpin->setEnabled(!b);
		m_rollingModeSw->setEnabled(!b);
		if(b) {
			m_rollingModeSw->onOffswitch()->setChecked(false);
			m_plotSizeSpin->setValue(m_bufferSizeSpin->value());
			connect(m_bufferSizeSpin, &ScaleSpinButton::valueChanged, m_plotSizeSpin,
				&ScaleSpinButton::setValue);
		} else {
			disconnect(m_bufferSizeSpin, &ScaleSpinButton::valueChanged, m_plotSizeSpin,
				   &ScaleSpinButton::setValue);
		}
	});
	m_rollingModeSw = new MenuOnOffSwitch(tr("ROLLING MODE"), xaxis, false);
	connect(m_rollingModeSw->onOffswitch(), &QAbstractButton::toggled, this,
		&TimePlotManagerSettings::setRollingMode);

	QWidget *xMinMax = new QWidget(xaxis);
	QHBoxLayout *xMinMaxLayout = new QHBoxLayout(xMinMax);
	xMinMaxLayout->setMargin(0);
	xMinMaxLayout->setSpacing(10);
	xMinMax->setLayout(xMinMaxLayout);

	m_xmin = new PositionSpinButton(
		{
		 {"ns", 1E-9},
		 {"μs", 1E-6},
		 {"ms", 1E-3},
		 {"s", 1e0},
		 {"ks", 1e3},
		 {"Ms", 1e6},
		 {"Gs", 1e9},
		 },
		"XMin", -DBL_MAX, DBL_MAX, false, false, xMinMax);

	m_xmax = new PositionSpinButton(
		{
		 {"ns", 1E-9},
		 {"μs", 1E-6},
		 {"ms", 1E-3},
		 {"s", 1e0},
		 {"ks", 1e3},
		 {"Ms", 1e6},
		 {"Gs", 1e9},
		 },
		"XMax", -DBL_MAX, DBL_MAX, false, false, xMinMax);

	connect(m_xmin, &PositionSpinButton::valueChanged, this, [=](double min) {
		m_plotManager->setXInterval(m_xmin->value(), m_xmax->value());
	});
	connect(m_xmax, &PositionSpinButton::valueChanged, this, [=](double max) {
		m_plotManager->setXInterval(m_xmin->value(), m_xmax->value());
	});

	xMinMaxLayout->addWidget(m_xmin);
	xMinMaxLayout->addWidget(m_xmax);

	m_xModeCb = new MenuCombo("XMode", xaxis);
	auto cb = m_xModeCb->combo();

	cb->addItem("Samples", XMODE_SAMPLES);
	cb->addItem("Time - override samplerate", XMODE_OVERRIDE);

	connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		m_sampleRateSpin->setVisible(false);
		if(cb->itemData(idx) == XMODE_SAMPLES) {
			m_sampleRateSpin->setValue(1);
			// setMetricFormatter - xAxis
			// setUnits xmin,xmax - k,mega
		}
		if(cb->itemData(idx) == XMODE_TIME) {
			m_sampleRateSpin->setVisible(true);
			m_sampleRateSpin->setValue(readSampleRate());
			// setTimeFormatter - xAxis
			// setUnits xmin,xmax - time units
		}
		if(cb->itemData(idx) == XMODE_OVERRIDE) {
			m_sampleRateSpin->setVisible(true);
			m_sampleRateSpin->setEnabled(true);
			// setTimeFormatter - xAxis
			// setUnits xmin,xmax
		}
	});

	m_sampleRateSpin = new PositionSpinButton(
		{
		 {"Hz", 1e0},
		 {"kHz", 1e3},
		 {"MHz", 1e6},
		 {"GHz", 1e9},
		 },
		"SampleRate", 1, DBL_MAX, false, false, xaxis);

	m_sampleRateSpin->setValue(10);
	m_sampleRateSpin->setEnabled(false);
	connect(m_sampleRateSpin, &PositionSpinButton::valueChanged,
		this, [=](double val) {
			setSampleRate(val);
		});

	connect(this, &TimePlotManagerSettings::sampleRateChanged, m_sampleRateSpin, &PositionSpinButton::setValue);


	xaxiscontainer->contentLayout()->setSpacing(10);
	xaxiscontainer->contentLayout()->addWidget(xaxis);
	xaxis->contentLayout()->addWidget(bufferPlotSize);
	xaxis->contentLayout()->addWidget(m_syncBufferPlot);
	xaxis->contentLayout()->addWidget(m_rollingModeSw);
	xaxis->contentLayout()->addWidget(xMinMax);
	xaxis->contentLayout()->addWidget(m_xModeCb);
	xaxis->contentLayout()->addWidget(m_sampleRateSpin);

	xaxis->contentLayout()->setSpacing(10);

	return xaxiscontainer;
}

void TimePlotManagerSettings::onInit() {
	m_bufferSizeSpin->setValue(32);
	m_plotSizeSpin->setValue(32);
	m_sampleRateSpin->setValue(1);
	m_xmin->setValue(0);
	m_xmax->setValue(31);
	m_syncBufferPlot->onOffswitch()->setChecked(true);
	m_xModeCb->combo()->setCurrentIndex(0);

	       //	m_rollingModeSw->onOffswitch()->setChecked(false);
}


double TimePlotManagerSettings::sampleRate() const { return m_sampleRate; }

void TimePlotManagerSettings::setSampleRate(double newSampleRate)
{
	if(qFuzzyCompare(m_sampleRate, newSampleRate))
		return;
	m_sampleRate = newSampleRate;
	Q_EMIT sampleRateChanged(m_sampleRate);
	updateXAxis();
}

bool TimePlotManagerSettings::rollingMode() const { return m_rollingMode; }

void TimePlotManagerSettings::setRollingMode(bool newRollingMode)
{
	if(m_rollingMode == newRollingMode)
		return;
	m_rollingMode = newRollingMode;
	Q_EMIT rollingModeChanged(newRollingMode);
	updateXAxis();
}

uint32_t TimePlotManagerSettings::plotSize() const { return m_plotSize; }

void TimePlotManagerSettings::setPlotSize(uint32_t newPlotSize)
{
	if(m_plotSize == newPlotSize)
		return;
	m_plotSize = newPlotSize;
	Q_EMIT plotSizeChanged(newPlotSize);
	updateXAxis();
}

uint32_t TimePlotManagerSettings::bufferSize() const { return m_bufferSize; }

void TimePlotManagerSettings::setBufferSize(uint32_t newBufferSize)
{
	if(m_bufferSize == newBufferSize)
		return;
	m_bufferSize = newBufferSize;
	Q_EMIT bufferSizeChanged(newBufferSize);
	updateXAxis();
}

void TimePlotManagerSettings::updateXAxis() {

	double min = 0;
	double max = m_plotSize;

	min = min / m_sampleRate;
	max = max / m_sampleRate;

	if(m_rollingMode) {
		m_xmin->setValue(max);
		m_xmax->setValue(min);
	} else {
		m_xmin->setValue(min);
		m_xmax->setValue(max);
	}
}

void TimePlotManagerSettings::onStart()
{
	QComboBox *cb = m_xModeCb->combo();

	if(cb->itemData(cb->currentIndex()) == XMODE_TIME) {
		double sr = readSampleRate();
		setSampleRate(sr);
	} else {
		Q_EMIT sampleRateChanged(m_sampleRate);
	}

	Q_EMIT plotSizeChanged(m_plotSize);
	Q_EMIT rollingModeChanged(m_rollingMode);
	if(!m_syncMode) {
		Q_EMIT bufferSizeChanged(m_bufferSize);
	}
	updateXAxis();
}

bool TimePlotManagerSettings::syncBufferPlotSize() const { return m_syncBufferPlotSize; }

void TimePlotManagerSettings::setSyncBufferPlotSize(bool newSyncBufferPlotSize)
{
	if(m_syncBufferPlotSize == newSyncBufferPlotSize)
		return;
	m_syncBufferPlotSize = newSyncBufferPlotSize;
	Q_EMIT syncBufferPlotSizeChanged(newSyncBufferPlotSize);
}

void TimePlotManagerSettings::addPlot(TimePlotComponent *plt) {
	QWidget *plotMenu = plt->plotMenu();
	m_plotContainerLayout->addWidget(plotMenu);
}

void TimePlotManagerSettings::removePlot(TimePlotComponent *p) {
	/*m_plotContainerLayout->removeWidget(p->plotMenu());
	p->deletePlotMenu();*/
}

void TimePlotManagerSettings::addChannel(ChannelComponent *c) {
	m_channels.append(c);
	// autoscaler->addChannels(c->plotCh());/
}

void TimePlotManagerSettings::removeChannel(ChannelComponent *c) {
	m_channels.removeAll(c);
	// autoscaler->removeChannels(c->plotCh());
}

void TimePlotManagerSettings::addSampleRateProvider(SampleRateProvider *s) {
	enableXModeTime();
	m_sampleRateProviders.append(s);
}

void TimePlotManagerSettings::removeSampleRateProvider(SampleRateProvider *s) {
	m_sampleRateProviders.removeAll(s);
}

void TimePlotManagerSettings::enableXModeTime() {
	if(m_sampleRateAvailable) // already set
		return;
	m_sampleRateAvailable = true;
	if(m_sampleRateAvailable) {
		auto cb = m_xModeCb->combo();
		cb->insertItem(1,"Time", XMODE_TIME);
	}
}
double TimePlotManagerSettings::readSampleRate()
{
	double sr = 1;
	for(SampleRateProvider *ch : qAsConst(m_sampleRateProviders)) {
		if(ch->sampleRateAvailable()) {
			sr = ch->sampleRate();
			break;
		}
	}
	return sr;
}


} // namespace adc
} // namespace scopy
