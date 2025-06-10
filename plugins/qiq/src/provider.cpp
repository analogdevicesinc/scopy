#include "provider.h"
#include "plotaxis.h"
#include <qtconcurrentrun.h>

#include <stylehelper.h>
#include <tooltemplate.h>

using namespace scopy::qiqplugin;

Provider::Provider(QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	setLayout(layout);

	m_dataAcq = new DataAcquisition(this);
	m_dataProvider = new DataProvider(this);
	m_dataProvider->setScriptPath(QString(QIQPLUGIN_RES_PATH) + QDir::separator() + "processData1.py");

	m_runBtn = new RunBtn(this);

	setupPlotWidget();
	setupToolTemplate();
	setupConnections();
}

Provider::~Provider() {}

// void Provider::onDataAcqAvailable(const double *data, const int &dataSize, const QString &path)
// {
// 	size_t samplesPerChannel = (dataSize / sizeof(double)) / 2;
// 	QVector<QVector<double>> vData(CHNL_NUMBER);
// 	for(int i = 0; i < samplesPerChannel; i++) {
// 		for(int j = 0; j < CHNL_NUMBER; j++) {
// 			vData[j].push_back(*(data + i * CHNL_NUMBER + j));
// 		}
// 	}
// 	for(int i = 0; i < vData.size(); i++) {
// 		m_acqPlot->getChannels()[i]->curve()->setSamples(m_xValues, vData[i]);
// 	}
// 	m_acqPlot->replot();

// 	m_dataProvider->processData(path);
// }

void Provider::onDataAcqAvailable(const short *data, const int &dataSize, const QString &path)
{
	DebugTimer acqPlot("/home/andrei/Desktop/benchmark.csv");
	size_t samplesPerChannel = (dataSize / sizeof(short)) / 2;
	QVector<QVector<double>> vData(CHNL_NUMBER);
	for(int i = 0; i < samplesPerChannel; i++) {
		for(int j = 0; j < CHNL_NUMBER; j++) {
			vData[j].push_back(*(data + i * CHNL_NUMBER + j));
		}
	}
	DEBUGTIMER_LOG(acqPlot, "Structure data for plotting:");
	acqPlot.restartTimer();
	for(int i = 0; i < vData.size(); i++) {
		m_acqPlot->getChannels()[i]->curve()->setSamples(m_xValues, vData[i]);
	}
	m_acqPlot->replot();
	DEBUGTIMER_LOG(acqPlot, "Plot device samples:");

	m_dataProvider->processData(path);
}

void Provider::setupPlotWidget()
{
	m_acqPlot = new PlotWidget(this);
	configurePlot(m_acqPlot, -200, 200);

	m_receiverPlot = new PlotWidget(this);
	configurePlot(m_receiverPlot, -200, 200);
}

void Provider::setupToolTemplate()
{
	ToolTemplate *tool = new ToolTemplate(this);
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(true);
	tool->centralContainer()->setVisible(true);
	tool->topContainerMenuControl()->setVisible(false);
	tool->rightContainer()->setVisible(false);

	tool->addWidgetToCentralContainerHelper(m_acqPlot);
	tool->addWidgetToCentralContainerHelper(m_receiverPlot);
	tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);

	layout()->addWidget(tool);
}

void Provider::setupConnections()
{
	connect(m_dataAcq, &DataAcquisition::dataAvailable, this, &Provider::onDataAcqAvailable);

	connect(m_dataProvider, &DataProvider::dataReady, this, [this](const QVector<QVector<double>> &processedData) {
		DebugTimer timer("/home/andrei/Desktop/benchmark.csv");
		for(int i = 0; i < processedData.size(); i++) {
			m_receiverPlot->getChannels()[i]->curve()->setSamples(m_xValues, processedData[i]);
		}
		m_receiverPlot->replot();
		DEBUGTIMER_LOG(timer, "Plot processed samples:");
		if(m_runBtn->isChecked()) {
			m_dataAcq->readDeviceData();
		}
		DEBUGTIMER_LOG(m_testTimer, "Total:");
		m_testTimer.restartTimer();
	});

	connect(m_runBtn, &RunBtn::toggled, this, [this](bool en) {
		if(en) {
			m_testTimer.startTimer();
			m_dataAcq->readDeviceData();
		}
	});
}

void Provider::configurePlot(PlotWidget *plot, int yMin, int yMax)
{
	int samplesPerChnl = SAMPLES_PER_CHANNEL;
	plot->xAxis()->setInterval(0, samplesPerChnl);

	plot->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	plot->yAxis()->scaleDraw()->setFloatPrecision(2);
	plot->yAxis()->setInterval(yMin, yMax);

	plot->setShowXAxisLabels(true);
	plot->setShowYAxisLabels(true);

	addPlotChannel(plot, "ch0", StyleHelper::getChannelColor(0));
	addPlotChannel(plot, "ch1", StyleHelper::getChannelColor(1));

	if(samplesPerChnl != m_xValues.size()) {
		m_xValues.clear();
		for(int i = 0; i < samplesPerChnl; i++) {
			m_xValues.push_back(i);
		}
	}

	plot->replot();
}

void Provider::addPlotChannel(PlotWidget *plot, const QString &label, const QColor &color)
{
	QPen pen(color, 1);
	PlotChannel *channel = new PlotChannel(label, pen, plot->xAxis(), plot->yAxis(), this);
	plot->addPlotChannel(channel);
	channel->setEnabled(true);
}
