#include "provider.h"
#include "plotaxis.h"
#include <qtconcurrentrun.h>

#include <stylehelper.h>
#include <tooltemplate.h>

#include <iioutil/connectionprovider.h>

using namespace scopy::qiqplugin;

Provider::Provider(QString uri, QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	setLayout(layout);

	Connection *conn = ConnectionProvider::GetInstance()->open(uri);

	m_acqSetup = new AcqSetup(this);
	m_dataAcq = new DataAcq(conn->context(), this);

	m_dataProvider = new DataProvider(this);
	// m_dataProvider->setScriptPath(QString(QIQPLUGIN_RES_PATH) + QDir::separator() + "processData1.py");

	m_runBtn = new RunBtn(this);
	m_runBtn->setDisabled(true);

	setupPlotWidget();
	setupToolTemplate();
	setupConnections();

	m_dataAcq->initDevicesAndChnls();
}

Provider::~Provider() { ConnectionProvider::GetInstance()->close(m_uri); }

void Provider::onDataAcqAvailable(QVector<QVector<double>> data, const int &dataSize, const QString &path)
{
	DebugTimer acqPlot(scopy::config::settingsFolderPath() + QDir::separator() + "benchmark.csv");

	DEBUGTIMER_LOG(acqPlot, "Structure data for plotting:");
	acqPlot.restartTimer();
	for(int i = 0; i < data.size(); i++) {
		m_acqPlot->getChannels()[i]->curve()->setSamples(m_xValues, data[i]);
	}
	m_acqPlot->replot();
	DEBUGTIMER_LOG(acqPlot, "Plot device samples:");
	m_dataProvider->processData(path);
}

void Provider::initXAxis(PlotWidget *plot, int samples)
{
	plot->xAxis()->setInterval(0, samples);
	if(samples != m_xValues.size()) {
		m_xValues.clear();
		for(int i = 0; i < samples; i++) {
			m_xValues.push_back(i);
		}
	}
	plot->replot();
}

void Provider::setupPlotWidget()
{
	m_acqPlot = new PlotWidget(this);
	configurePlot(m_acqPlot, -200, 200);

	m_receiverPlot = new PlotWidget(this);
	configurePlot(m_receiverPlot, -200, 200);
	addPlotChannel(m_receiverPlot, "ch0", StyleHelper::getChannelColor(0));
}

void Provider::setupToolTemplate()
{
	ToolTemplate *tool = new ToolTemplate(this);
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(true);
	tool->centralContainer()->setVisible(true);
	tool->topContainerMenuControl()->setVisible(false);
	tool->rightContainer()->setVisible(false);

	tool->addWidgetToCentralContainerHelper(m_acqSetup);
	tool->addWidgetToCentralContainerHelper(m_acqPlot);
	tool->addWidgetToCentralContainerHelper(m_receiverPlot);
	tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);

	layout()->addWidget(tool);
}

void Provider::setupConnections()
{
	connect(m_dataAcq, &DataAcq::dataAvailable, this, &Provider::onDataAcqAvailable);

	connect(m_dataProvider, &DataProvider::dataReady, this, [this](const QVector<QVector<double>> &processedData) {
		DebugTimer timer(scopy::config::settingsFolderPath() + QDir::separator() + "benchmark.csv");
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
	connect(m_dataProvider, &DataProvider::stopAcq, this, [this]() { m_runBtn->setChecked(false); });
	connect(m_runBtn, &RunBtn::toggled, m_acqSetup, &AcqSetup::setDisabled);
	connect(m_acqSetup, &AcqSetup::configPressed, this, &Provider::configPressed);
	connect(m_dataAcq, &DataAcq::contextInfo, m_acqSetup, &AcqSetup::init);
}

void Provider::removePlotChannels(PlotWidget *plot)
{
	const QList<PlotChannel *> chnls = plot->getChannels();
	for(PlotChannel *ch : chnls) {
		plot->removePlotChannel(ch);
		delete ch;
	}
	plot->replot();
}

void Provider::initChannels(PlotWidget *plot, QStringList chnls)
{
	removePlotChannels(plot);
	int i = 0;
	for(const QString &ch : chnls) {
		addPlotChannel(plot, ch, StyleHelper::getChannelColor(i));
		i++;
	}
	plot->replot();
}

void Provider::configPressed(AcqSetup::AcqConfig config)
{
	initChannels(m_acqPlot, config.enChnls);
	initXAxis(m_acqPlot, config.samples);
	initXAxis(m_receiverPlot, config.samples);
	m_dataAcq->onConfigPressed(config);
	if(!config.cliPath.isEmpty()) {
		m_dataProvider->setCliPath(config.cliPath);
	}
	m_dataProvider->runProcess(config.enChnls.size());
	m_runBtn->setEnabled(true);
}

void Provider::configurePlot(PlotWidget *plot, int yMin, int yMax)
{
	initXAxis(plot, 1024);

	plot->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	plot->yAxis()->scaleDraw()->setFloatPrecision(2);
	plot->yAxis()->setInterval(yMin, yMax);

	plot->setShowXAxisLabels(true);
	plot->setShowYAxisLabels(true);

	plot->replot();
}

void Provider::addPlotChannel(PlotWidget *plot, const QString &label, const QColor &color)
{
	QPen pen(color, 1);
	PlotChannel *channel = new PlotChannel(label, pen, plot->xAxis(), plot->yAxis(), this);
	plot->addPlotChannel(channel);
	channel->setEnabled(true);
}
