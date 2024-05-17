#include "waveforminstrument.h"
#include "plotaxis.h"
#include <qwt_legend.h>
#include <gui/stylehelper.h>
#include <gui/widgets/verticalchannelmanager.h>
#include <gui/plotaxis.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuheader.h>

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
	tool->rightContainer()->setVisible(true);
	tool->setRightContainerWidth(280);
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

	m_settBtn = new GearBtn(this);
	m_settBtn->setChecked(true);
	tool->rightStack()->add("settings", createSettMenu(this));
	connect(m_settBtn, &QPushButton::toggled, this, [=, this](bool b) { tool->openRightContainerHelper(b); });

	tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(m_settBtn, TTA_RIGHT);

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
	m_xTime.clear();
	for(int i = m_plotSampleRate - 1; i >= 0; i--) {
		m_xTime.push_back(-(i / m_plotSampleRate));
	}
	for(const QMap<QString, QString> &chMap : m_chnls) {
		for(const QString &ch : chMap) {
			m_yValues[ch] = QVector<double>();
		}
	}
}

void WaveformInstrument::initPlot(PlotWidget *plot, QString unitType, int yMin, int yMax)
{
	plot->plot()->insertLegend(new QwtLegend(), QwtPlot::TopLegend);
	plot->xAxis()->setInterval(-1, 0);

	plot->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	plot->yAxis()->scaleDraw()->setFloatPrecision(2);
	plot->yAxis()->scaleDraw()->setUnitType(unitType);
	plot->yAxis()->setInterval(yMin, yMax);

	plot->setShowXAxisLabels(true);
	plot->setShowYAxisLabels(true);
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
		m_plotChnls[chnlId] = plotCh;
		chnlIdx++;
	}
}

QWidget *WaveformInstrument::createSettMenu(QWidget *parent)
{

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setMargin(0);
	layout->setSpacing(10);

	MenuHeaderWidget *header = new MenuHeaderWidget("Settings", QPen(StyleHelper::getColor("ScopyBlue")), widget);
	MenuSectionWidget *plotSettingsContainer = new MenuSectionWidget(widget);
	MenuCollapseSection *plotTimespanSection =
		new MenuCollapseSection("PLOT", MenuCollapseSection::MHCW_NONE, widget);
	plotTimespanSection->setLayout(new QVBoxLayout());
	plotTimespanSection->contentLayout()->setSpacing(10);
	plotTimespanSection->contentLayout()->setMargin(0);

	// timespan
	m_timespanSpin = new PositionSpinButton({{"ms", 1E-3}, {"s", 1E0}}, "Timespan", 0.1, 10, true, false);
	m_timespanSpin->setStep(0.1);
	m_timespanSpin->setValue(1);
	connect(m_timespanSpin, &PositionSpinButton::valueChanged, this, [=, this](double value) {
		m_voltagePlot->xAxis()->setMin(-value);
		m_currentPlot->xAxis()->setMin(-value);
	});

	plotTimespanSection->contentLayout()->addWidget(m_timespanSpin);

	plotSettingsContainer->contentLayout()->addWidget(plotTimespanSection);
	layout->addWidget(header);
	layout->addWidget(plotSettingsContainer);
	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	return widget;
}

void WaveformInstrument::stop() { m_runBtn->setChecked(false); }

void WaveformInstrument::toggleWaveform(bool en)
{
	if(en) {
		ResourceManager::open("pqm", this);
	} else {
		ResourceManager::close("pqm");
	}
	const QStringList keys = m_yValues.keys();
	for(const QString &chnlId : keys) {
		m_yValues[chnlId].clear();
	}
	Q_EMIT enableTool(en);
}

void WaveformInstrument::updateXData(int dataSize)
{
	double timespanValue = m_timespanSpin->value();
	double plotSamples = m_plotSampleRate * timespanValue;
	if(m_xTime.size() == plotSamples && dataSize == plotSamples) {
		return;
	}
	m_xTime.clear();
	for(int i = dataSize - 1; i >= 0; i--) {
		m_xTime.push_back(-(i / plotSamples) * timespanValue);
	}
}

void WaveformInstrument::plotData(QVector<double> chnlData, QString chnlId)
{
	int dataSize = chnlData.size();
	updateXData(dataSize);
	m_plotChnls[chnlId]->curve()->setSamples(m_xTime.data(), chnlData.data(), dataSize);
	m_voltagePlot->replot();
	m_currentPlot->replot();
}

void WaveformInstrument::onBufferDataAvailable(QMap<QString, QVector<double>> data)
{
	if(m_runBtn->isChecked() || m_singleBtn->isChecked()) {

		int samplingFreq = m_plotSampleRate * m_timespanSpin->value();
		const QStringList keys = data.keys();
		for(const auto &key : keys) {
			m_yValues[key].append(data[key]);
			if(m_yValues[key].size() > samplingFreq) {
				int unnecessarySamples = m_yValues[key].size() - samplingFreq;
				m_yValues[key].erase(m_yValues[key].begin(),
						     m_yValues[key].begin() + unnecessarySamples);
			}
			plotData(m_yValues[key], key);
		}
		if(m_singleBtn->isChecked() && m_yValues.first().size() == samplingFreq) {
			m_singleBtn->setChecked(false);
		}
	}
}

#include "moc_waveforminstrument.cpp"
