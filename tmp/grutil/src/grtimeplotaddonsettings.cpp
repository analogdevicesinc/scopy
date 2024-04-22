#include "grtimeplotaddonsettings.h"

#include "grtimechanneladdon.h"
#include "grdeviceaddon.h"
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
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>

using namespace scopy::grutil;
using namespace scopy::gui;

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
	QScrollArea *scroll = new QScrollArea(parent);
	scroll->setWidgetResizable(true);
	QWidget *w = new QWidget(scroll);
	scroll->setWidget(w);
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setMargin(0);
	lay->setSpacing(10);
	w->setLayout(lay);

	m_pen = QPen(StyleHelper::getColor("ScopyBlue"));

	MenuHeaderWidget *header = new MenuHeaderWidget("PLOT", m_pen, w);
	QWidget *xaxismenu = createXAxisMenu(w);
	QWidget *yaxismenu = createYAxisMenu(w);
	QWidget *fftmenu = createFFTMenu(w);
	QWidget *xymenu = createXYMenu(w);
	//	QWidget* curvemenu = createCurveMenu(w);

	lay->addWidget(header);
	lay->addWidget(xaxismenu);
	lay->addWidget(yaxismenu);
	lay->addWidget(fftmenu);
	lay->addWidget(xymenu);
	//	lay->addWidget(curvemenu);

	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	return scroll;
}

QWidget *GRTimePlotAddonSettings::createYAxisMenu(QWidget *parent)
{
	MenuSectionWidget *yaxiscontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *yaxis = new MenuCollapseSection("Y-AXIS", MenuCollapseSection::MHCW_NONE, yaxiscontainer);

	m_plot->plot()->yAxis()->setUnits("V");
	m_yctrl = new MenuPlotAxisRangeControl(m_plot->plot()->yAxis(), yaxis);
	m_singleYModeSw = new MenuOnOffSwitch("Single Y Mode", yaxis);
	m_autoscaleBtn = new QPushButton("Autoscale", yaxis);

	autoscaler = new PlotAutoscaler(false, this);
	connect(autoscaler, &PlotAutoscaler::newMin, m_yctrl, &MenuPlotAxisRangeControl::setMin);
	connect(autoscaler, &PlotAutoscaler::newMax, m_yctrl, &MenuPlotAxisRangeControl::setMax);
	StyleHelper::BlueButton(m_autoscaleBtn, "autoscale");

	connect(m_autoscaleBtn, &QPushButton::clicked, this, [=]() { autoscaler->autoscale(); });

	yaxis->contentLayout()->addWidget(m_singleYModeSw);
	yaxis->contentLayout()->addWidget(m_yctrl);
	yaxis->contentLayout()->addSpacerItem(new QSpacerItem(0, 5, QSizePolicy::Fixed, QSizePolicy::Fixed));
	yaxis->contentLayout()->addWidget(m_autoscaleBtn);
	yaxiscontainer->contentLayout()->addWidget(yaxis);

	connect(m_singleYModeSw->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_yctrl->setEnabled(b);
		m_autoscaleBtn->setEnabled(b);
		for(auto ch : channels) {
			SingleYModeAware *singleyaware = dynamic_cast<SingleYModeAware *>(ch);
			if(singleyaware) {
				singleyaware->setSingleYMode(b);
			}
		}
		m_plot->replot();
	});

	return yaxiscontainer;
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
		"Buffer Size", 16, DBL_MAX, false, false, bufferPlotSize);

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
		"Plot Size", 16, DBL_MAX, false, false, bufferPlotSize);

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
		"SampleRate", 1, DBL_MAX, false, false, xaxis);

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

QWidget *GRTimePlotAddonSettings::createXYMenu(QWidget *parent)
{
	MenuSectionWidget *xycontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *xy = new MenuCollapseSection("X-Y", MenuCollapseSection::MHCW_NONE, xycontainer);
	QVBoxLayout *xyMenuLayout = new QVBoxLayout();
	xyMenuLayout->setMargin(0);
	xyMenuLayout->setSpacing(10);
	QVBoxLayout *xyLayout = new QVBoxLayout();
	xyLayout->setSpacing(10);
	xyLayout->setMargin(0);

	xy_xaxis = new MenuCombo("X-Axis", xy);
	xy_xaxis->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	xy_yaxis = new MenuCombo("Y-Axis", xy);
	xy_yaxis->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

	connect(xy_xaxis->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		int xindex = -1;
		int yindex = -1;
		for(int i = 0; i < channels.count(); i++) {
			QString channelName = getComboNameFromChannelHelper(channels[i]);
			if(channelName == xy_xaxis->combo()->currentText()) {
				xindex = i;
			}
			if(channelName == xy_yaxis->combo()->currentText()) {
				yindex = i;
			}
		}

		if(xindex != -1 && yindex != -1) {
			m_plot->setXYSource(dynamic_cast<GRTimeChannelAddon *>(channels[xindex]),
					    dynamic_cast<GRTimeChannelAddon *>(channels[yindex]));
		}
	});

	connect(xy_yaxis->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		int xindex = -1;
		int yindex = -1;
		for(int i = 0; i < channels.count(); i++) {
			QString channelName = getComboNameFromChannelHelper(channels[i]);
			if(channelName == xy_xaxis->combo()->currentText()) {
				xindex = i;
			}
			if(channelName == xy_yaxis->combo()->currentText()) {
				yindex = i;
			}
		}

		if(xindex != -1 && yindex != -1) {
			m_plot->setXYSource(dynamic_cast<GRTimeChannelAddon *>(channels[xindex]),
					    dynamic_cast<GRTimeChannelAddon *>(channels[yindex]));
		}
	});

	xyLayout->addWidget(xy_xaxis);
	xyLayout->addWidget(xy_yaxis);

	auto curve = createCurveMenu(m_plot->xyplotch(), xy);

	xycontainer->contentLayout()->addWidget(xy);
	xy->contentLayout()->addLayout(xyMenuLayout);
	xyMenuLayout->addLayout(xyLayout);
	xyMenuLayout->addWidget(curve);

	return xycontainer;
}

QWidget *GRTimePlotAddonSettings::createCurveMenu(PlotChannel *ch, QWidget *parent)
{
	MenuCollapseSection *curve = new MenuCollapseSection("CURVE", MenuCollapseSection::MHCW_NONE, parent);

	MenuPlotChannelCurveStyleControl *curveSettings = new MenuPlotChannelCurveStyleControl(curve);
	curveSettings->addChannels(ch);

	curve->contentLayout()->addWidget(curveSettings);
	return curve;
}

QWidget *GRTimePlotAddonSettings::createFFTMenu(QWidget *parent)
{
	MenuSectionWidget *fftcontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *fft = new MenuCollapseSection("FFT", MenuCollapseSection::MHCW_NONE, fftcontainer);

	QVBoxLayout *fftLayout = new QVBoxLayout();
	fftLayout->setMargin(0);
	fftLayout->setSpacing(10);

	QVBoxLayout *iqLayout = new QVBoxLayout();
	iqLayout->setMargin(0);
	iqLayout->setSpacing(10);

	fft_i = new MenuCombo("FFT Real", fft);
	fft_i->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	fft_q = new MenuCombo("FFT Imag", fft);
	fft_q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

	fft_complex_mode = new MenuOnOffSwitch("ComplexMode", fft, true);
	fft_complex_mode->onOffswitch()->setChecked(false);
	fft_q->setVisible(false);
	connect(fft_complex_mode->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		fft_q->setVisible(b);
		Q_EMIT fftComplexModeChanged(b);
	});

	connect(fft_i->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		int iindex = -1;
		int qindex = -1;
		for(int i = 0; i < channels.count(); i++) {
			QString channelName = getComboNameFromChannelHelper(channels[i]);
			if(channelName == fft_i->combo()->currentText()) {
				iindex = i;
			}
			if(channelName == fft_q->combo()->currentText()) {
				qindex = i;
			}
		}

		if(iindex != -1 && qindex != -1)
			m_plot->setFFTSource(dynamic_cast<GRTimeChannelAddon *>(channels[iindex]),
					     dynamic_cast<GRTimeChannelAddon *>(channels[qindex]));
	});

	connect(fft_q->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		int iindex = -1;
		int qindex = -1;
		for(int i = 0; i < channels.count(); i++) {
			QString channelName = getComboNameFromChannelHelper(channels[i]);
			if(channelName == fft_i->combo()->currentText()) {
				iindex = i;
			}
			if(channelName == fft_q->combo()->currentText()) {
				qindex = i;
			}
		}

		if(iindex != -1 && qindex != -1) {
			m_plot->setFFTSource(dynamic_cast<GRTimeChannelAddon *>(channels[iindex]),
					     dynamic_cast<GRTimeChannelAddon *>(channels[qindex]));
		}
	});

	m_freqOffsetSpin = new PositionSpinButton(
		{
			{"Hz", 1e0},
			{"kHz", 1e3},
			{"MHz", 1e6},
			{"GHz", 1e9},
		},
		"Frequency Offset", 0, DBL_MAX, false, false, fft);

	m_freqOffsetSpin->setEnabled(true);
	connect(m_freqOffsetSpin, &PositionSpinButton::valueChanged, this, [=](double val) { setFreqOffset(val); });
	connect(this, &GRTimePlotAddonSettings::freqOffsetChanged, m_freqOffsetSpin, &PositionSpinButton::setValue);
	connect(this, &GRTimePlotAddonSettings::freqOffsetChanged, m_plot, &GRTimePlotAddon::setFreqOffset);
	connect(this, &GRTimePlotAddonSettings::fftComplexModeChanged, m_plot, &GRTimePlotAddon::setComplexMode);

	fftLayout->addWidget(fft_complex_mode);
	fftLayout->addLayout(iqLayout);
	iqLayout->addWidget(fft_i);
	iqLayout->addWidget(fft_q);

	auto m_fftwindow = new MenuCombo("Window", fft);
	m_fftwindow->combo()->addItem("Hamming", 0);
	m_fftwindow->combo()->addItem("Hann", 1);
	m_fftwindow->combo()->addItem("Blackman", 2);
	m_fftwindow->combo()->addItem("Rectangular", 3);
	m_fftwindow->combo()->addItem("Blackman-Harris", 5);
	m_fftwindow->combo()->addItem("Bartlett", 6);
	m_fftwindow->combo()->addItem("Flattop", 7);
	//	m_fftwindow->combo()->addItem("None",-1);

	connect(m_fftwindow->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this,
		[=](int idx) { m_plot->setFftWindow(m_fftwindow->combo()->itemData(idx).toInt()); });

	fftLayout->addWidget(m_fftwindow);
	fftLayout->addWidget(m_freqOffsetSpin);

	// m_plot->fftplotch()->yAxis()->setUnits("db");
	m_fftyctrl = new MenuPlotAxisRangeControl(m_plot->fftplotch()->yAxis(), fft);
	fftLayout->addWidget(m_fftyctrl);

	auto curve = createCurveMenu(m_plot->fftplotch(), fft);
	fftLayout->addWidget(curve);

	fftcontainer->contentLayout()->addWidget(fft);
	fft->contentLayout()->addLayout(fftLayout);
	return fftcontainer;
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
	m_yctrl->setEnabled(false);
	m_singleYModeSw->setEnabled(true);
	m_singleYModeSw->onOffswitch()->setChecked(false);
	m_autoscaleBtn->setEnabled(false);
	m_fftyctrl->setMax(5);
	m_fftyctrl->setMin(-145);

	fft_complex_mode->onOffswitch()->setChecked(false);
	//	m_rollingModeSw->onOffswitch()->setChecked(false);
}

void GRTimePlotAddonSettings::onDeinit() {}

QString GRTimePlotAddonSettings::getComboNameFromChannelHelper(ChannelAddon *t)
{
	GRTimeChannelAddon *grch = dynamic_cast<GRTimeChannelAddon *>(t);
	QString suffix = "";
	if(grch) {
		suffix = " @ " + grch->getDevice()->getName();
	}

	return t->getName() + suffix;
}

void GRTimePlotAddonSettings::onChannelAdded(ChannelAddon *t)
{
	channels.append(t);
	autoscaler->addChannels(t->plotCh());

	QString channelName = getComboNameFromChannelHelper(t);

	xy_xaxis->combo()->addItem(channelName);
	xy_yaxis->combo()->addItem(channelName);

	fft_i->combo()->addItem(channelName);
	fft_q->combo()->addItem(channelName);
}

void GRTimePlotAddonSettings::onChannelRemoved(ChannelAddon *t)
{
	channels.removeAll(t);
	autoscaler->addChannels(t->plotCh());

	QString channelName = getComboNameFromChannelHelper(t);
	int idx = 0;
	for(idx = 0; xy_xaxis->combo()->itemText(idx) != channelName; idx++)
		;
	xy_xaxis->combo()->removeItem(idx);
	for(idx = 0; xy_xaxis->combo()->itemText(idx) != channelName; idx++)
		;
	xy_yaxis->combo()->removeItem(idx);

	for(idx = 0; xy_xaxis->combo()->itemText(idx) != channelName; idx++)
		;
	fft_i->combo()->removeItem(idx);
	for(idx = 0; xy_xaxis->combo()->itemText(idx) != channelName; idx++)
		;
	fft_q->combo()->removeItem(idx);
}

double GRTimePlotAddonSettings::readSampleRate()
{
	double sr = 1;
	for(ChannelAddon *ch : channels) {
		if(!ch->enabled())
			continue;
		SampleRateProvider *srp = dynamic_cast<SampleRateProvider *>(ch);
		if(!srp)
			continue;
		sr = srp->sampleRate();
		break;
	}
	return sr;
}

double GRTimePlotAddonSettings::freqOffset() const { return m_freqOffset; }

void GRTimePlotAddonSettings::setFreqOffset(double newFreqOffset)
{
	if(qFuzzyCompare(m_freqOffset, newFreqOffset))
		return;
	m_freqOffset = newFreqOffset;
	emit freqOffsetChanged(newFreqOffset);
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
	for(ChannelAddon *ch : channels) {
		if(!ch->enabled())
			continue;
		SampleRateProvider *srp = dynamic_cast<SampleRateProvider *>(ch);
		if(!srp)
			continue;
		if(srp->sampleRateAvailable()) {
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

#include "moc_grtimeplotaddonsettings.cpp"
