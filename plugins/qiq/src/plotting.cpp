#include "plotting.h"

#include <QLoggingCategory>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QProcess>
#include <QFile>
#include <qtconcurrentrun.h>

#include <style.h>
#include <stylehelper.h>
#include <tooltemplate.h>
#include <plotaxis.h>

using namespace scopy::qiqplugin;

Plotting::Plotting(QWidget *parent)
	: QWidget(parent)
	, m_data(nullptr)
	, m_readFw(new QFutureWatcher<void>(this))
	, m_timer(QTimer(this))
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	setLayout(layout);

	setupButtons();
	setupPlotWidget();
	setupToolTemplate();
	setupConnections();

	m_timer.setInterval(50);
	runPython(QStringList() << "--init"
				<< "--path" << SHARED_FILE);
}

Plotting::~Plotting()
{
	unmapFile();
	if(m_readFw->isRunning()) {
		m_readFw->cancel();
	}
}

void Plotting::setupButtons()
{
	QButtonGroup *btnGroup = new QButtonGroup(this);
	btnGroup->setExclusive(true);

	m_rqstBtn = createButton("Request samples");
	m_stopBtn = createButton("Stop");

	btnGroup->addButton(m_rqstBtn);
	btnGroup->addButton(m_stopBtn);
}

QPushButton *Plotting::createButton(const QString &text)
{
	QPushButton *btn = new QPushButton(text, this);
	Style::setStyle(btn, style::properties::button::basicButton);
	btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	btn->setCheckable(true);
	btn->setDisabled(true);
	return btn;
}

void Plotting::setupPlotWidget()
{
	m_receiverPlot = new PlotWidget(this);
	configurePlot(m_receiverPlot, -200, 200);
}

void Plotting::setupToolTemplate()
{
	ToolTemplate *tool = new ToolTemplate(this);
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(true);
	tool->centralContainer()->setVisible(true);
	tool->topContainerMenuControl()->setVisible(false);
	tool->rightContainer()->setVisible(false);

	tool->addWidgetToTopContainerHelper(m_stopBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(m_rqstBtn, TTA_RIGHT);
	tool->addWidgetToCentralContainerHelper(m_receiverPlot);

	layout()->addWidget(tool);
}

void Plotting::setupConnections()
{
	connect(m_readFw, &QFutureWatcher<void>::finished, this, [this]() {
		if(m_init) {
			m_init = false;
			mapFile(SHARED_FILE);
			m_rqstBtn->setEnabled(true);
			m_stopBtn->setEnabled(true);
		}
	});

	connect(m_rqstBtn, &QPushButton::pressed, this, [this]() {
		m_timer.start();
		runPython(QStringList() << "--path" << SHARED_FILE);
	});

	connect(m_stopBtn, &QPushButton::pressed, this, [this]() {
		m_timer.stop();
		if(m_readFw->isRunning()) {
			m_readFw->cancel();
		}
	});

	connect(&m_timer, &QTimer::timeout, this, [this]() { plotData(SAMPLES); });
}

void Plotting::configurePlot(PlotWidget *plot, int yMin, int yMax)
{
	plot->xAxis()->setInterval(0, SAMPLES);

	plot->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	plot->yAxis()->scaleDraw()->setFloatPrecision(2);
	plot->yAxis()->setInterval(yMin, yMax);

	plot->setShowXAxisLabels(true);
	plot->setShowYAxisLabels(true);

	addPlotChannel(plot, "ch0", StyleHelper::getChannelColor(0));
	addPlotChannel(plot, "ch1", StyleHelper::getChannelColor(1));

	plot->replot();
}

void Plotting::addPlotChannel(PlotWidget *plot, const QString &label, const QColor &color)
{
	QPen pen(color, 1);
	PlotChannel *channel = new PlotChannel(label, pen, plot->xAxis(), plot->yAxis(), this);
	plot->addPlotChannel(channel);
	channel->setEnabled(true);
}

void Plotting::plotData(int maxSamples)
{
	QFile lockFile(READ_LOCK);
	if(lockFile.exists()) {
		qInfo() << "Writer is active. Waiting...";
		return;
	}

	QFile writeLock(WRITE_LOCK);
	writeLock.open(QIODevice::ReadWrite);

	if(!m_data || m_size < 4) {
		qWarning() << "Invalid file";
		writeLock.remove();
		return;
	}

	int numSamples = m_size / sizeof(Sample);
	maxSamples = std::min(maxSamples, numSamples);

	const Sample *samples = reinterpret_cast<const Sample *>(m_data);

	QVector<double> x, ch0Y, ch1Y;
	x.reserve(maxSamples);
	ch0Y.reserve(maxSamples);
	ch1Y.reserve(maxSamples);

	for(int i = 0; i < maxSamples; ++i) {
		x.append(i);
		ch0Y.append(samples[i].ch1);
		ch1Y.append(samples[i].ch2);
	}

	m_receiverPlot->getChannels()[0]->curve()->setSamples(x, ch0Y);
	m_receiverPlot->getChannels()[1]->curve()->setSamples(x, ch1Y);
	m_receiverPlot->replot();

	writeLock.remove();
}

bool Plotting::mapFile(const QString &path)
{
	m_file.setFileName(path);
	if(!m_file.open(QIODevice::ReadOnly)) {
		qInfo() << "Cannot open file: " << path;
		return false;
	}
	m_size = m_file.size();
	m_data = m_file.map(0, m_size);
	m_file.close();
	return true;
}

void Plotting::unmapFile()
{
	if(m_data) {
		m_file.unmap(m_data);
		m_data = nullptr;
	}
}

void Plotting::runPython(const QStringList args)
{
	if(m_readFw->isRunning()) {
		return;
	}

	QFuture<void> f = QtConcurrent::run([this, args]() {
		QString program = "python3";

		QProcess process;
		process.start(program, QStringList() << SCRIPT << args);

		if(!process.waitForStarted()) {
			qDebug() << "Running error!";
			return;
		}

		process.waitForFinished(30000);

		qInfo() << "Output:" << process.readAllStandardOutput();
		qInfo() << "Errors:" << process.readAllStandardError();
	});
	m_readFw->setFuture(f);
}
