#include "waveforminstrument.h"
#include "plotaxis.h"
#include <gui/stylehelper.h>
#include <gui/widgets/verticalchannelmanager.h>
#include <gui/plotaxis.h>

using namespace scopy::pqm;

WaveformInstrument::WaveformInstrument(QWidget *parent)
	: QWidget(parent)
{
	initData();
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *layout = new QHBoxLayout(this);
	setLayout(layout);
	StyleHelper::GetInstance()->initColorMap();

	ToolTemplate *tool = new ToolTemplate(this);
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(true);
	tool->centralContainer()->setVisible(true);
	tool->topContainerMenuControl()->setVisible(false);
	layout->addWidget(tool);

	m_voltagePlot = new PlotWidget(this);
	initPlot(m_voltagePlot, "V", -400, 400);
	setupChannels(m_voltagePlot, m_chnls["voltage"]);
	tool->addWidgetToCentralContainerHelper(m_voltagePlot);

	m_currentPlot = new PlotWidget(this);
	initPlot(m_currentPlot, "A", -20, 20);
	setupChannels(m_currentPlot, m_chnls["current"]);
	tool->addWidgetToCentralContainerHelper(m_currentPlot);

	m_runBtn = new RunBtn(this);
	m_singleBtn = new SingleShotBtn(this);

	tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);

	connect(this, &WaveformInstrument::runTme, m_runBtn, &QAbstractButton::setChecked);
	connect(m_runBtn, &QAbstractButton::toggled, m_singleBtn, &QAbstractButton::setDisabled);
	connect(m_runBtn, SIGNAL(toggled(bool)), this, SLOT(toggleWaveform(bool)));
	connect(m_singleBtn, &QAbstractButton::toggled, m_runBtn, &QAbstractButton::setDisabled);
	connect(m_singleBtn, SIGNAL(toggled(bool)), this, SLOT(toggleWaveform(bool)));
}

WaveformInstrument::~WaveformInstrument()
{
	m_xTime.clear();
	m_yValues.clear();
}

void WaveformInstrument::initData()
{
	for(int i = 0; i < SAMPLE_RATE; i++) {
		m_xTime.push_back((i / (double)SAMPLE_RATE));
	}
	for(const QMap<QString, QString> &chMap : m_chnls) {
		for(const QString &ch : chMap) {
			m_yValues[ch] = std::vector<double>(SAMPLE_RATE, 0);
		}
	}
}

void WaveformInstrument::initPlot(PlotWidget *plot, QString unitType, int yMin, int yMax)
{
	plot->plot()->insertLegend(new QwtLegend(), QwtPlot::TopLegend);
	plot->xAxis()->setInterval(XMIN, XMAX);
	plot->xAxis()->setVisible(true);
	plot->yAxis()->setInterval(yMin, yMax);
	plot->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	plot->yAxis()->scaleDraw()->setFloatPrecision(2);
	plot->yAxis()->scaleDraw()->setUnitType(unitType);
	plot->yAxis()->setVisible(true);
	plot->replot();
}

void WaveformInstrument::setupChannels(PlotWidget *plot, QMap<QString, QString> chnls)
{
	int chnlIdx = 0;
	for(const QString &chnlId : chnls) {
		QPen chPen = QPen(QColor(StyleHelper::getColor("CH" + QString::number(chnlIdx))), 1);
		PlotChannel *plotCh = new PlotChannel(chnls.key(chnlId), chPen, plot->xAxis(), plot->yAxis(), this);
		plot->addPlotChannel(plotCh);
		plotCh->setEnabled(true);
		plotCh->curve()->setRawSamples(m_xTime.data(), m_yValues[chnlId].data(), m_xTime.size());
		chnlIdx++;
	}
}

void WaveformInstrument::stop() { m_runBtn->setChecked(false); }

void WaveformInstrument::toggleWaveform(bool en)
{
	if(en) {
		ResourceManager::open("pqm", this);
	} else {
		ResourceManager::close("pqm");
	}
	Q_EMIT enableTool(en);
}

void WaveformInstrument::onBufferDataAvailable(QMap<QString, std::vector<double>> data)
{
	if(m_runBtn->isChecked() || m_singleBtn->isChecked()) {
		const QStringList chList = m_yValues.keys();
		for(const QString &ch : chList) {
			m_yValues[ch].clear();
			m_yValues[ch] = data[ch];
		}
		m_voltagePlot->replot();
		m_currentPlot->replot();
		if(m_singleBtn->isChecked()) {
			m_singleBtn->setChecked(false);
		}
	}
}
