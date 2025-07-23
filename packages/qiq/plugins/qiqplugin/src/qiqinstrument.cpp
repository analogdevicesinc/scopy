#include "qiqinstrument.h"
#include "plotaxis.h"

#include <plotfactory.h>
#include <stylehelper.h>
#include <tooltemplate.h>

using namespace scopy::qiqplugin;

QIQInstrument::QIQInstrument(QWidget *parent)
	: QWidget(parent)
	, m_cliProcess(new QProcess)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);

	ToolTemplate *tool = new ToolTemplate(this);
	tool->topContainer()->setVisible(true);
	tool->centralContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	tool->setRightContainerWidth(280);

	QWidget *centralW = createCentralWidget(tool);
	m_plotManager = new PlotManager(this);

	m_settings = new SettingsMenu(this);
	GearBtn *settingsBtn = new GearBtn(this);
	settingsBtn->setChecked(true);

	m_runBtn = new RunBtn(this);
	m_runBtn->setDisabled(true);

	tool->addWidgetToCentralContainerHelper(centralW);
	tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(settingsBtn, TTA_RIGHT);
	tool->rightStack()->add("settings", m_settings);

	layout->addWidget(tool);

	setupConnections();
	connect(m_runBtn, &RunBtn::toggled, this, &QIQInstrument::runPressed);
	connect(settingsBtn, &QPushButton::toggled, this, [=, this](bool b) { tool->openRightContainerHelper(b); });

	// to be deleted
	connect(m_cliProcess, &QProcess::readyReadStandardOutput, this, [this]() {
		QString output = m_cliProcess->readAllStandardOutput();
		if(output.contains("FAIL")) {
			m_runBtn->setChecked(false);
		}
		if(output.contains("OK")) {
			m_plotManager->onDataIsProcessed(0, m_xAxis.size());
		}
	});
	connect(m_cliProcess, &QProcess::readyReadStandardError, this, [this]() {
		QString output = m_cliProcess->readAllStandardError();
		qInfo() << "Error:" << output;
	});
}

QIQInstrument::~QIQInstrument()
{
	if(m_cliProcess) {
		m_cliProcess->kill();
	}
}

void QIQInstrument::processData()
{
	QString cmd("p " + QString::number(m_inputPlot->getChannels().size()));
	m_cliProcess->write(cmd.toStdString().c_str());
	m_cliProcess->write("\n");
}

void QIQInstrument::setAvailableChannels(QMap<QString, QStringList> channels)
{
	m_settings->setAvailableChannels(channels);
}

void QIQInstrument::onAnalysisTypes(const QStringList &types) { m_settings->setAnalysisTypes(types); }

void QIQInstrument::onInputFormatChanged(const InputConfig &inConfig)
{
	updateXAxis(inConfig.sampleCount(), inConfig.samplingFrequency());
	updateChannels(inConfig.channelCount());
	m_plotManager->samplingFreqAvailable(inConfig.samplingFrequency());
	m_runBtn->setEnabled(inConfig.channelCount() > 0);
}

void QIQInstrument::onAnalysisInfo(const QString &type, const QVariantMap &params, const OutputInfo &outputInfo,
				   const QList<QIQPlotInfo> plotInfoList)
{
	m_settings->setAnalysisParams(type, params);
	m_plotManager->onAvailableInfo(outputInfo, plotInfoList);
	addPlots();
}

void QIQInstrument::onAnalysisConfigured(const QString &type, const QVariantMap &config, const OutputInfo &outputInfo)
{}

// TBD
void QIQInstrument::addPlots()
{
	const QList<QWidget *> plotList = m_plotManager->getPlotW();
	int row = 0, col = 0;
	if(m_plotsLay->count() == 1) {
		row = 1;
	}
	for(QWidget *p : plotList) {
		m_plotsLay->addWidget(p, row, col);
		row++;
		if(row > 1) {
			col++;
			row = 0;
		}
	}
}

void QIQInstrument::removePlots()
{
	const QList<QWidget *> plotList = m_plotManager->getPlotW();
	for(QWidget *p : plotList) {
		m_plotsLay->removeWidget(p);
	}
}

void QIQInstrument::setupConnections()
{
	connect(m_settings, &SettingsMenu::analysisChanged, this, &QIQInstrument::requestAnalysisInfo);
	connect(m_settings, &SettingsMenu::analysisConfig, this, &QIQInstrument::analysisConfigChanged);
	connect(m_settings, &SettingsMenu::bufferParamsChanged, this, &QIQInstrument::bufferParamsChanged);
	connect(this, &QIQInstrument::bufferDataReady, this, [this](QVector<QVector<double>> data) {
		if(data.isEmpty()) {
			return;
		}
		if(!m_cliProcess->isOpen()) {
			runProcess();
		}
		int i = 0;
		for(PlotChannel *ch : m_inputPlot->getChannels()) {
			ch->curve()->setSamples(m_xAxis, data[i]);
			i++;
		}
		m_inputPlot->replot();
		processData();
	});
	connect(m_plotManager, &PlotManager::requestNewData, this, &QIQInstrument::requestNewData);
}

QWidget *QIQInstrument::createCentralWidget(QWidget *parent)
{
	QWidget *central = new QWidget(parent);
	central->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_plotsLay = new QGridLayout(central);
	m_plotsLay->setMargin(0);
	central->setLayout(m_plotsLay);
	createInputPlot();

	return central;
}

void QIQInstrument::createInputPlot()
{
	m_inputPlot = new PlotWidget();
	m_inputPlot->plot()->setTitle("Input waveform");
	m_inputPlot->xAxis()->setUnits("Time");
	m_inputPlot->yAxis()->setUnits("Magnitude");

	updateChannels(1);

	m_inputPlot->xAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_inputPlot->xAxis()->scaleDraw()->setFloatPrecision(2);
	updateXAxis(1024, 1024);

	m_inputPlot->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_inputPlot->yAxis()->scaleDraw()->setFloatPrecision(2);
	m_inputPlot->yAxis()->setInterval(-200, 200);

	m_inputPlot->setShowXAxisLabels(true);
	m_inputPlot->setShowYAxisLabels(true);

	m_inputPlot->replot();

	m_plotsLay->addWidget(m_inputPlot, 0, 0);
}

void QIQInstrument::updateXAxis(int samples, int sampleRate)
{
	if(m_xAxis.size() == samples) {
		return;
	}
	m_xAxis.clear();
	for(int i = 0; i < samples; i++) {
		m_xAxis.push_back((double)i / sampleRate);
	}
	m_inputPlot->xAxis()->setInterval(0, (double)samples / sampleRate);
	m_inputPlot->replot();
}

void QIQInstrument::addPlotChannel(const QString &label, const QColor &color)
{
	QPen pen(color, 1);
	PlotChannel *channel = new PlotChannel(label, pen, m_inputPlot->xAxis(), m_inputPlot->yAxis(), this);
	m_inputPlot->addPlotChannel(channel);
	channel->setEnabled(true);
}

void QIQInstrument::removePlotChannels()
{
	const QList<PlotChannel *> chnls = m_inputPlot->getChannels();
	for(PlotChannel *ch : chnls) {
		m_inputPlot->removePlotChannel(ch);
		delete ch;
	}
	m_inputPlot->replot();
}

void QIQInstrument::updateChannels(int chnlCount)
{
	removePlotChannels();
	for(int i = 0; i < chnlCount; i++) {
		addPlotChannel("ch" + QString::number(i), StyleHelper::getChannelColor(i));
	}
	m_inputPlot->replot();
}

// to be deleted

void QIQInstrument::runProcess()
{

	if(m_cliProcess->state() == QProcess::Running) {
		m_cliProcess->kill();
		m_cliProcess->waitForFinished();
	}

	QString program =
		"/home/andrei/git_repositories/scopy/process_data/build/Desktop_Qt_5_15_2_GCC_64bit-Debug/process_data";
	m_cliProcess->start(program,
			    QStringList() << scopy::config::executableFolderPath() + QDir::separator() + "data.in"
					  << scopy::config::executableFolderPath() + QDir::separator() + "data.out");

	if(!m_cliProcess->waitForStarted()) {
		qWarning() << "Running C error!";
		return;
	}

	qInfo() << "Process started";
	qInfo() << m_cliProcess->readAllStandardOutput();
	qInfo() << m_cliProcess->readAllStandardError();
}
