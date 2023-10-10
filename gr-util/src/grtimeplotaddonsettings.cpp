#include "grtimeplotaddonsettings.h"

#include "grtimechanneladdon.h"
#include "plotaxis.h"
#include "plotwidget.h"

#include <QComboBox>
#include <QVBoxLayout>

#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuheader.h>
#include <gui/widgets/menulineedit.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menusectionwidget.h>

using namespace scopy::grutil;

GRTimePlotAddonSettings::GRTimePlotAddonSettings(GRTimePlotAddon *p, QObject *parent)
	: QObject(parent)
	, m_plot(p)
{
	name = p->getName() + "_settings";
	m_sampleRateAvailable = true;
	widget = createMenu();
}

GRTimePlotAddonSettings::~GRTimePlotAddonSettings() {}

QWidget *GRTimePlotAddonSettings::createMenu(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setMargin(0);
	lay->setSpacing(10);
	w->setLayout(lay);

	m_pen = QPen(StyleHelper::getColor("ScopyBlue"));

	MenuHeaderWidget *header = new MenuHeaderWidget("PLOT", m_pen, w);
	QWidget *xaxismenu = createXAxisMenu(w);
	//	QWidget* curvemenu = createCurveMenu(w);

	lay->addWidget(header);
	lay->addWidget(xaxismenu);
	//	lay->addWidget(curvemenu);

	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	return w;
}

QWidget *GRTimePlotAddonSettings::createXAxisMenu(QWidget *parent)
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
		"Buffer Size", 16, (double)((long)1 << 31), false, false, bufferPlotSize);

	connect(m_bufferSizeSpin, &ScaleSpinButton::valueChanged, this, [=](double val) {
		if(m_plotSizeSpin->value() < val) {
			m_plotSizeSpin->setValue(val);
		}
		m_plotSizeSpin->setMinValue(val);
		setBufferSize((uint32_t)val);
	});
	connect(this, &GRTimePlotAddonSettings::bufferSizeChanged, m_plot, &GRTimePlotAddon::setBufferSize);

	m_plotSizeSpin = new ScaleSpinButton(
		{
			{"samples", 1e0},
			{"ksamples", 1e3},
			{"Msamples", 1e6},
			{"Gsamples", 1e9},
		},
		"Plot Size", 16, (double)((long)1 << 31), false, false, bufferPlotSize);

	connect(m_plotSizeSpin, &ScaleSpinButton::valueChanged, this, [=](double val) { setPlotSize((uint32_t)val); });
	connect(this, &GRTimePlotAddonSettings::plotSizeChanged, m_plot, &GRTimePlotAddon::setPlotSize);

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
		&GRTimePlotAddonSettings::setRollingMode);
	connect(this, &GRTimePlotAddonSettings::rollingModeChanged, m_plot, &GRTimePlotAddon::setRollingMode);

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
		"XMin", (double)((-((long)1 << 31))), (double)((long)1 << 31), false, false, xMinMax);

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
		"XMax", (double)((-((long)1 << 31))), (double)((long)1 << 31), false, false, xMinMax);

	auto m_plotAxis = m_plot->plot()->xAxis();
	// Connects
	connect(m_xmin, &PositionSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMin);
	connect(m_plotAxis, &PlotAxis::minChanged, this, [=]() {
		QSignalBlocker b(m_xmin);
		m_xmin->setValue(m_plotAxis->min());
		m_plot->updateBufferPreviewer();
	});

	connect(m_xmax, &PositionSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMax);
	connect(m_plotAxis, &PlotAxis::maxChanged, this, [=]() {
		QSignalBlocker b(m_xmax);
		m_xmax->setValue(m_plotAxis->max());
		m_plot->updateBufferPreviewer();
	});

	xMinMaxLayout->addWidget(m_xmin);
	xMinMaxLayout->addWidget(m_xmax);

	m_xModeCb = new MenuCombo("XMode", xaxis);
	auto cb = m_xModeCb->combo();

	cb->addItem("Samples", XMODE_SAMPLES);
	if(m_sampleRateAvailable) {
		cb->addItem("Time", XMODE_TIME);
	}
	cb->addItem("Time - override samplerate", XMODE_OVERRIDE);

	connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		if(cb->itemData(idx) == XMODE_SAMPLES) {
			m_sampleRateSpin->setEnabled(false);
			m_sampleRateSpin->setValue(1);
			m_plot->setXMode(XMODE_SAMPLES);
			// setMetricFormatter - xAxis
			// setUnits xmin,xmax - k,mega
		}
		if(cb->itemData(idx) == XMODE_TIME) {
			m_sampleRateSpin->setEnabled(false);
			m_sampleRateSpin->setValue(readSampleRate());
			m_plot->setXMode(XMODE_TIME);
			// setTimeFormatter - xAxis
			// setUnits xmin,xmax - time units
		}
		if(cb->itemData(idx) == XMODE_OVERRIDE) {
			m_sampleRateSpin->setEnabled(true);
			m_plot->setXMode(XMODE_TIME);
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
		"SampleRate", 1, (double)((long)1 << 31), false, false, xaxis);

	m_sampleRateSpin->setEnabled(false);
	connect(m_sampleRateSpin, &PositionSpinButton::valueChanged, this, [=](double val) { setSampleRate(val); });
	connect(this, &GRTimePlotAddonSettings::sampleRateChanged, m_sampleRateSpin, &PositionSpinButton::setValue);
	connect(this, &GRTimePlotAddonSettings::sampleRateChanged, m_plot, &GRTimePlotAddon::setSampleRate);

	m_showTagsSw = new MenuOnOffSwitch(tr("SHOW TAGS"), xaxis, false);
	connect(m_showTagsSw->onOffswitch(), &QAbstractButton::toggled, this,
		&GRTimePlotAddonSettings::setShowPlotTags);
	connect(this, &GRTimePlotAddonSettings::showPlotTagsChanged, m_plot, &GRTimePlotAddon::setDrawPlotTags);

	m_showLabels = new MenuOnOffSwitch("PLOT LABELS", xaxis);
	showPlotLabels(false);
	connect(m_showLabels->onOffswitch(), &QAbstractButton::toggled, this, &GRTimePlotAddonSettings::showPlotLabels);

	xaxiscontainer->contentLayout()->setSpacing(10);
	xaxiscontainer->contentLayout()->addWidget(xaxis);
	xaxis->contentLayout()->addWidget(bufferPlotSize);
	xaxis->contentLayout()->addWidget(m_syncBufferPlot);
	xaxis->contentLayout()->addWidget(m_rollingModeSw);
	xaxis->contentLayout()->addWidget(xMinMax);
	xaxis->contentLayout()->addWidget(m_xModeCb);
	xaxis->contentLayout()->addWidget(m_sampleRateSpin);
	xaxis->contentLayout()->addWidget(m_showTagsSw);
	xaxis->contentLayout()->addWidget(m_showLabels);
	xaxis->contentLayout()->setSpacing(10);

	return xaxiscontainer;
}

double GRTimePlotAddonSettings::sampleRate() const { return m_sampleRate; }

void GRTimePlotAddonSettings::setSampleRate(double newSampleRate)
{
	if(qFuzzyCompare(m_sampleRate, newSampleRate))
		return;
	m_sampleRate = newSampleRate;
	Q_EMIT sampleRateChanged(m_sampleRate);
}

bool GRTimePlotAddonSettings::showPlotTags() const { return m_showPlotTags; }

void GRTimePlotAddonSettings::setShowPlotTags(bool newShowPlotTags)
{
	if(m_showPlotTags == newShowPlotTags)
		return;
	m_showPlotTags = newShowPlotTags;
	Q_EMIT showPlotTagsChanged(m_showPlotTags);
}

bool GRTimePlotAddonSettings::rollingMode() const { return m_rollingMode; }

void GRTimePlotAddonSettings::setRollingMode(bool newRollingMode)
{
	if(m_rollingMode == newRollingMode)
		return;
	m_rollingMode = newRollingMode;
	Q_EMIT rollingModeChanged(newRollingMode);
}

uint32_t GRTimePlotAddonSettings::plotSize() const { return m_plotSize; }

void GRTimePlotAddonSettings::setPlotSize(uint32_t newPlotSize)
{
	if(m_plotSize == newPlotSize)
		return;
	m_plotSize = newPlotSize;
	Q_EMIT plotSizeChanged(newPlotSize);
}

void GRTimePlotAddonSettings::onInit()
{
	m_bufferSizeSpin->setValue(32);
	m_plotSizeSpin->setValue(32);
	m_xmin->setValue(0);
	m_xmax->setValue(31);
	m_syncBufferPlot->onOffswitch()->setChecked(true);
	m_showLabels->onOffswitch()->setChecked(false);
	m_xModeCb->combo()->setCurrentIndex(0);
	//	m_rollingModeSw->onOffswitch()->setChecked(false);
}

void GRTimePlotAddonSettings::onDeinit() {}

void GRTimePlotAddonSettings::onChannelAdded(ToolAddon *t)
{
	auto ch = dynamic_cast<GRTimeChannelAddon *>(t);
	if(ch)
		grChannels.append(ch);
}

void GRTimePlotAddonSettings::onChannelRemoved(ToolAddon *t)
{
	auto ch = dynamic_cast<GRTimeChannelAddon *>(t);
	if(ch)
		grChannels.removeAll(ch);
}

double GRTimePlotAddonSettings::readSampleRate()
{
	double sr = 1;
	for(GRTimeChannelAddon *gr : grChannels) {
		if(!gr->enabled())
			continue;
		sr = gr->grch()->readSampleRate();
		break;
	}
	return sr;
}

void GRTimePlotAddonSettings::preFlowBuild()
{
	QComboBox *cb = m_xModeCb->combo();

	if(cb->itemData(cb->currentIndex()) != XMODE_TIME)
		return;

	double sr = readSampleRate();
	setSampleRate(sr);
}

uint32_t GRTimePlotAddonSettings::bufferSize() const { return m_bufferSize; }

void GRTimePlotAddonSettings::setBufferSize(uint32_t newBufferSize)
{
	if(m_bufferSize == newBufferSize)
		return;
	m_bufferSize = newBufferSize;
	Q_EMIT bufferSizeChanged(newBufferSize);
}

void GRTimePlotAddonSettings::computeSampleRateAvailable()
{
	bool sampleRateAvailable = false;
	for(GRTimeChannelAddon *gr : grChannels) {
		if(!gr->enabled())
			continue;
		if(gr->sampleRateAvailable()) {
			sampleRateAvailable = true;
			break;
		}
	}
}

void GRTimePlotAddonSettings::showPlotLabels(bool b)
{
	PlotWidget *plotWidget = m_plot->plot();
	plotWidget->setShowXAxisLabels(b);
	plotWidget->setShowYAxisLabels(b);
	plotWidget->showAxisLabels();
}

QString GRTimePlotAddonSettings::getName() { return name; }

QWidget *GRTimePlotAddonSettings::getWidget() { return widget; }
