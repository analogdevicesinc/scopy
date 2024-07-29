#include "fftplotmanagersettings.h"
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuheader.h>
#include <gui/stylehelper.h>
#include <grtimechannelcomponent.h>

namespace scopy {
namespace adc {

FFTPlotManagerSettings::FFTPlotManagerSettings(FFTPlotManager *mgr, QWidget *parent)
	: QWidget(parent)
	, ToolComponent()
	, m_sampleRateAvailable(false)
{
	m_plotManager = mgr;
	auto *w = createMenu(this);
	QVBoxLayout *lay = new QVBoxLayout(parent);
	lay->addWidget(w);
	lay->setSpacing(0);
	lay->setMargin(0);
	setLayout(lay);
	m_samplingInfo.bufferSize = 32;
	m_samplingInfo.plotSize = 32;
	m_samplingInfo.complexMode = false;
	m_samplingInfo.freqOffset = 0;
	m_samplingInfo.sampleRate = 1;
	m_samplingInfo.startingPoint = 0;

}

FFTPlotManagerSettings::~FFTPlotManagerSettings() {}

QWidget *FFTPlotManagerSettings::createMenu(QWidget *parent)
{
	m_pen = QPen(StyleHelper::getColor("ScopyBlue"));
	m_menu = new MenuWidget("FFT PLOT", m_pen, parent);

	QWidget *xaxismenu = createXAxisMenu(m_menu);

	m_addPlotBtn = new QPushButton("Add Plot", this);
	StyleHelper::BlueButton(m_addPlotBtn, "AddPlotButton");

	connect(m_addPlotBtn, &QPushButton::clicked, this, [=]() {
		uint32_t idx = m_plotManager->addPlot("Plot ");
		FFTPlotComponent *plt = m_plotManager->plot(idx);
		addPlot(plt);
	});

	connect(m_plotManager, &PlotManager::plotRemoved, this, [=](uint32_t uuid) {
		FFTPlotComponent *plt = m_plotManager->plot(uuid);
		removePlot(plt);
	});

	m_plotSection = new MenuSectionWidget(this);
	m_plotCb = new MenuCombo("Plot Settings", m_plotSection);
	m_plotSection->contentLayout()->addWidget(m_plotCb);

	m_plotStack = new MapStackedWidget(this);
	m_plotStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_menu->add(xaxismenu, "xaxis");
	m_menu->add(m_plotSection);
	m_menu->add(m_plotStack);

	connect(m_plotCb->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		m_plotStack->show(QString(m_plotCb->combo()->currentData().toInt()));
	});

	m_menu->add(m_addPlotBtn, "add", gui::MenuWidget::MA_BOTTOMLAST);

	return m_menu;
}

QWidget *FFTPlotManagerSettings::createXAxisMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *section =
		new MenuSectionCollapseWidget("X-AXIS", MenuCollapseSection::MHCW_NONE, parent);

	m_bufferSizeSpin = new ScaleSpinButton(
		{
		 {"samples", 1e0},
		 {"ksamples", 1e3},
		 {"Msamples", 1e6},
		 },
		"Buffer Size", 16, DBL_MAX, false, false, section);

	connect(m_bufferSizeSpin, &ScaleSpinButton::valueChanged, this, [=](double val) {
		setBufferSize((uint32_t)val);
	});


	QWidget *xMinMax = new QWidget(section);
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

	connect(m_xmin, &PositionSpinButton::valueChanged, this,
		[=](double min) { m_plotManager->setXInterval(m_xmin->value(), m_xmax->value()); });
	connect(m_xmax, &PositionSpinButton::valueChanged, this,
		[=](double max) { m_plotManager->setXInterval(m_xmin->value(), m_xmax->value()); });

	xMinMaxLayout->addWidget(m_xmin);
	xMinMaxLayout->addWidget(m_xmax);

	m_xModeCb = new MenuCombo("XMode", section);
	auto xcb = m_xModeCb->combo();

	xcb->addItem("Samples", XMODE_SAMPLES);
	xcb->addItem("Time - override samplerate", XMODE_OVERRIDE);

	connect(xcb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		m_sampleRateSpin->setVisible(false);
		if(xcb->itemData(idx) == XMODE_SAMPLES) {
			m_sampleRateSpin->setValue(1);
			for(PlotComponent *plt : m_plotManager->plots()) {
				auto p = dynamic_cast<FFTPlotComponent*>(plt);
				p->fftPlot()->xAxis()->scaleDraw()->setFloatPrecision(0);
			}

		}
		if(xcb->itemData(idx) == XMODE_TIME) {
			m_sampleRateSpin->setVisible(true);
			m_sampleRateSpin->setEnabled(false);
			m_sampleRateSpin->setValue(readSampleRate());
			m_freqOffsetSpin->setVisible(true);
			m_freqOffsetSpin->setEnabled(true);

			for(PlotComponent *plt : m_plotManager->plots()) {
				auto p = dynamic_cast<FFTPlotComponent*>(plt);
				p->fftPlot()->xAxis()->scaleDraw()->setFloatPrecision(2);
			}

		}
		if(xcb->itemData(idx) == XMODE_OVERRIDE) {
			m_sampleRateSpin->setVisible(true);
			m_sampleRateSpin->setEnabled(true);
			m_freqOffsetSpin->setVisible(true);
			m_freqOffsetSpin->setEnabled(true);
			for(PlotComponent *plt : m_plotManager->plots()) {
				auto p = dynamic_cast<FFTPlotComponent*>(plt);
				p->fftPlot()->xAxis()->scaleDraw()->setFloatPrecision(2);
			}
		}
	});

	m_sampleRateSpin = new PositionSpinButton(
		{
		 {"Hz", 1e0},
		 {"kHz", 1e3},
		 {"MHz", 1e6},
		 {"GHz", 1e9},
		 },
		"SampleRate", 1, DBL_MAX, false, false, section);

	m_sampleRateSpin->setValue(10);
	m_sampleRateSpin->setEnabled(false);
	connect(m_sampleRateSpin, &PositionSpinButton::valueChanged, this, [=](double val) { setSampleRate(val); });

	m_freqOffsetSpin = new PositionSpinButton(
		{
		 {"Hz", 1e0},
		 {"kHz", 1e3},
		 {"MHz", 1e6},
		 {"GHz", 1e9},
		 },
		"Frequency Offset", 0, DBL_MAX, false, false, section);

	m_freqOffsetSpin->setValue(0);
	m_freqOffsetSpin->setEnabled(false);
	connect(m_freqOffsetSpin, &PositionSpinButton::valueChanged, this, [=](double val) { setFreqOffset(val); });
	connect(this, &FFTPlotManagerSettings::samplingInfoChanged, this, [=](SamplingInfo p){
		m_freqOffsetSpin->setValue(m_samplingInfo.freqOffset);
		m_sampleRateSpin->setValue(m_samplingInfo.sampleRate);
		m_bufferSizeSpin->setValue(m_samplingInfo.bufferSize);

	});

	section->contentLayout()->setSpacing(10);

	section->contentLayout()->addWidget(m_bufferSizeSpin);
	section->contentLayout()->addWidget(xMinMax);
	section->contentLayout()->addWidget(m_xModeCb);
	section->contentLayout()->addWidget(m_sampleRateSpin);
	section->contentLayout()->addWidget(m_freqOffsetSpin);
	section->contentLayout()->setSpacing(10);

	return section;
}

void FFTPlotManagerSettings::onInit()
{
	m_bufferSizeSpin->setValue(32);
	m_sampleRateSpin->setValue(1);
	m_xmin->setValue(0);
	m_xmax->setValue(31);
	m_xModeCb->combo()->setCurrentIndex(0);

	       //	m_rollingModeSw->onOffswitch()->setChecked(false);
}

void FFTPlotManagerSettings::updateXAxis()
{

	double min = 0;
	double max = 0;

	QComboBox *cb = m_xModeCb->combo();


	if(cb->itemData(cb->currentIndex()) == XMODE_TIME) {
		max = m_samplingInfo.sampleRate;
	} else {
		max = m_samplingInfo.bufferSize;
	}

	if(m_samplingInfo.complexMode) {
		min = -max;
	} else {
		min = 0;
	}

	min = m_samplingInfo.freqOffset + min/2;
	max = m_samplingInfo.freqOffset + max/2;

	m_xmin->setValue(min);
	m_xmax->setValue(max);

}

MenuWidget *FFTPlotManagerSettings::menu() { return m_menu; }

void FFTPlotManagerSettings::onStart()
{
	QComboBox *cb = m_xModeCb->combo();

	if(cb->itemData(cb->currentIndex()) == XMODE_TIME) {
		double sr = readSampleRate();
		setSampleRate(sr);
	}

	Q_EMIT samplingInfoChanged(m_samplingInfo);
	updateXAxis();
}

void FFTPlotManagerSettings::addPlot(FFTPlotComponent *p)
{
	QWidget *plotMenu = p->plotMenu();

	connect(p, &FFTPlotComponent::nameChanged, this, [=](QString newName){
		int idx = m_plotCb->combo()->findData(p->uuid());
		m_plotCb->combo()->setItemText(idx,newName);
	});
	m_plotCb->combo()->addItem(p->name(), p->uuid());
	m_plotStack->add(QString(p->uuid()), plotMenu);
	// m_menu->add(plotMenu, p->name() + QString(p->uuid()), gui::MenuWidget::MA_TOPLAST);
	setPlotComboVisible();
}

void FFTPlotManagerSettings::setPlotComboVisible() {
	bool visible = m_plotCb->combo()->count() > 1;
	m_plotSection->setVisible(visible);
}

void FFTPlotManagerSettings::removePlot(FFTPlotComponent *p)
{
	QWidget *plotMenu = p->plotMenu();
	// m_menu->remove(plotMenu);
	int idx = m_plotCb->combo()->findData(p->uuid());
	m_plotCb->combo()->removeItem(idx);
	m_plotStack->remove(QString(p->uuid()));

	setPlotComboVisible();
}

void FFTPlotManagerSettings::addChannel(ChannelComponent *c)
{
	m_channels.append(c);
	SampleRateProvider *srp = dynamic_cast<SampleRateProvider *>(c);
	if(srp) {
		addSampleRateProvider(srp);
	}
	c->setSamplingInfo(m_samplingInfo);
}

void FFTPlotManagerSettings::removeChannel(ChannelComponent *c)
{
	m_channels.removeAll(c);
	SampleRateProvider *srp = dynamic_cast<SampleRateProvider *>(c);
	if(srp) {
		removeSampleRateProvider(srp);
	}
}

void FFTPlotManagerSettings::addSampleRateProvider(SampleRateProvider *s)
{
	updateXModeCombo();
	m_sampleRateProviders.append(s);
}

void FFTPlotManagerSettings::removeSampleRateProvider(SampleRateProvider *s) { m_sampleRateProviders.removeAll(s); }

double FFTPlotManagerSettings::readSampleRate()
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


void FFTPlotManagerSettings::updateXModeCombo()
{
	if(m_sampleRateAvailable) // already set
		return;
	m_sampleRateAvailable = true;
	if(m_sampleRateAvailable) {
		auto cb = m_xModeCb->combo();
		cb->insertItem(1, "Time", XMODE_TIME);
	}
}


double FFTPlotManagerSettings::sampleRate() const { return m_samplingInfo.sampleRate; }

void FFTPlotManagerSettings::setSampleRate(double newSampleRate)
{
	if(qFuzzyCompare(m_samplingInfo.sampleRate, newSampleRate))
		return;
	m_samplingInfo.sampleRate = newSampleRate;
	Q_EMIT samplingInfoChanged(m_samplingInfo);
	updateXAxis();
}

uint32_t FFTPlotManagerSettings::bufferSize() const { return m_samplingInfo.bufferSize; }

void FFTPlotManagerSettings::setBufferSize(uint32_t newBufferSize)
{
	qInfo()<<"setBufferSize";
	if(m_samplingInfo.bufferSize == newBufferSize)
		return;
	m_samplingInfo.bufferSize = newBufferSize;
	Q_EMIT samplingInfoChanged(m_samplingInfo);
	updateXAxis();
}

double FFTPlotManagerSettings::freqOffset() const
{
	return m_samplingInfo.freqOffset;
}

void FFTPlotManagerSettings::setFreqOffset(double newFreqOffset)
{
	if (qFuzzyCompare(m_samplingInfo.freqOffset, newFreqOffset))
		return;
	m_samplingInfo.freqOffset = newFreqOffset;
	Q_EMIT samplingInfoChanged(m_samplingInfo);
	updateXAxis();
}

bool FFTPlotManagerSettings::complexMode() const
{
	return m_samplingInfo.complexMode;
}

void FFTPlotManagerSettings::setComplexMode(bool newComplexMode)
{
	if (m_samplingInfo.complexMode == newComplexMode)
		return;
	m_samplingInfo.complexMode = newComplexMode;
	Q_EMIT samplingInfoChanged(m_samplingInfo);
	updateXAxis();
}

/*void FFTPlotManagerSettings::collapseAllAndOpenMenu(QString s) {
	m_menu->collapseAll();
	m_menu->setCollapsed(s, true);
}*/

} // namespace adc
} // namespace scopy