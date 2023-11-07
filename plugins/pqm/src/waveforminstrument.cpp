#include "waveforminstrument.h"
#include "plotaxis.h"
#include <gui/stylehelper.h>
#include <gui/widgets/verticalchannelmanager.h>
#include <gui/plotaxis.h>
#include <fstream>

using namespace scopy::pqm;

WaveformInstrument::WaveformInstrument(QWidget *parent)
	: QWidget(parent)
{
	initData();
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *layout = new QHBoxLayout(this);
	setLayout(layout);
	StyleHelper::GetInstance()->initColorMap();

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	layout->addWidget(m_tool);

	m_voltagePlot = new PlotWidget(this);
	m_tool->addWidgetToCentralContainerHelper(m_voltagePlot);
	initPlot(m_voltagePlot, "V");
	setupChannels(m_voltagePlot, m_chnls["voltage"]);

	m_currentPlot = new PlotWidget(this);
	m_tool->addWidgetToCentralContainerHelper(m_currentPlot);
	initPlot(m_currentPlot, "A");
	setupChannels(m_currentPlot, m_chnls["current"]);

	m_runBtn = new RunBtn(this);
	m_singleBtn = new SingleShotBtn(this);

	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);

	connect(this, &WaveformInstrument::runTme, m_runBtn, &QAbstractButton::setChecked);
	connect(m_runBtn, &QAbstractButton::toggled, m_singleBtn, &QAbstractButton::setDisabled);
	connect(m_runBtn, SIGNAL(toggled(bool)), this, SIGNAL(enableTool(bool)));
	connect(m_singleBtn, &QAbstractButton::toggled, m_runBtn, &QAbstractButton::setDisabled);
	connect(m_singleBtn, SIGNAL(toggled(bool)), this, SIGNAL(enableTool(bool)));
}

WaveformInstrument::~WaveformInstrument()
{
	m_xTime.clear();
	m_yValues.clear();
}

void WaveformInstrument::setupChannels(PlotWidget *plot, QMap<QString, QString> chnls)
{
	int chnlIdx = 0;
	for(const QString &chnlId : chnls) {
		QPen chPen = QPen(QColor(StyleHelper::getColor("CH" + QString::number(chnlIdx))), 1);
		PlotChannel *plotCh =
			new PlotChannel(chnls.key(chnlId), chPen, plot, plot->xAxis(), plot->yAxis(), this);
		plotCh->setEnabled(true);
		plotCh->curve()->setRawSamples(m_xTime.data(), m_yValues[chnlId].data(), m_xTime.size());
		chnlIdx++;
	}
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

void WaveformInstrument::initPlot(PlotWidget *plot, QString unitType, int yMin, int yMax)
{
	plot->plot()->insertLegend(new QwtLegend(), QwtPlot::TopLegend);
	plot->xAxis()->setInterval(0, 1);
	plot->xAxis()->setVisible(true);
	plot->yAxis()->setInterval(yMin, yMax);
	plot->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	plot->yAxis()->scaleDraw()->setFloatPrecision(2);
	plot->yAxis()->scaleDraw()->setUnitType(unitType);
	plot->yAxis()->setVisible(true);
	plot->replot();
}
