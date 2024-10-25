/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "waveforminstrument.h"
#include "plotaxis.h"
#include "plottingstrategybuilder.h"
#include <QDate>
#include <QFileDialog>
#include <menulineedit.h>
#include <menuonoffswitch.h>
#include <plotnavigator.hpp>
#include <qwt_legend.h>
#include <rollingstrategy.h>
#include <swtriggerstrategy.h>
#include <gui/stylehelper.h>
#include <gui/widgets/verticalchannelmanager.h>
#include <gui/plotaxis.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuheader.h>

using namespace scopy::pqm;

WaveformInstrument::WaveformInstrument(ToolMenuEntry *tme, QString uri, QWidget *parent)
	: QWidget(parent)
	, m_tme(tme)
	, m_uri(uri)
	, m_running(false)
{
	initData();
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *layout = new QHBoxLayout(this);
	setLayout(layout);
	StyleHelper::GetInstance()->initColorMap();

	m_plottingStrategy = PlottingStrategyBuilder::build("trigger", m_plotSampleRate);
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

	PlotNavigator::syncPlotNavigators(m_voltagePlot->navigator(), m_currentPlot->navigator(),
					  new QSet<QwtAxisId>{m_voltagePlot->xAxis()->axisId()});

	m_runBtn = new RunBtn(this);
	m_singleBtn = new SingleShotBtn(this);

	m_settBtn = new GearBtn(this);
	m_settBtn->setChecked(true);
	tool->rightStack()->add("settings", createSettMenu(this));
	connect(m_settBtn, &QPushButton::toggled, this, [=, this](bool b) { tool->openRightContainerHelper(b); });

	tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(m_settBtn, TTA_RIGHT);

	connect(m_tme, &ToolMenuEntry::runClicked, m_runBtn, &QAbstractButton::setChecked);
	connect(this, &WaveformInstrument::enableTool, m_tme, &ToolMenuEntry::setRunning);
	connect(m_runBtn, &QAbstractButton::toggled, m_singleBtn, &QAbstractButton::setDisabled);
	connect(m_runBtn, SIGNAL(toggled(bool)), this, SLOT(toggleWaveform(bool)));
	connect(m_singleBtn, &QAbstractButton::toggled, m_runBtn, &QAbstractButton::setDisabled);
	connect(m_singleBtn, SIGNAL(toggled(bool)), this, SLOT(toggleWaveform(bool)));
}

WaveformInstrument::~WaveformInstrument() { m_xTime.clear(); }

void WaveformInstrument::showOneBuffer(bool hasFwVers)
{
	if(hasFwVers)
		return;
	m_timespanSpin->setEnabled(hasFwVers);
	m_timespanSpin->setValue(0.03);
}

void WaveformInstrument::initData()
{
	m_xTime.clear();
	for(int i = m_plotSampleRate - 1; i >= 0; i--) {
		m_xTime.push_back(-(i / m_plotSampleRate));
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
	QWidget *plotSection = createMenuPlotSection(widget);
	QWidget *logSection = createMenuLogSection(widget);

	layout->addWidget(header);
	layout->addWidget(plotSection);
	layout->addWidget(logSection);
	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	return widget;
}

QWidget *WaveformInstrument::createMenuPlotSection(QWidget *parent)
{
	MenuSectionCollapseWidget *plotSection = new MenuSectionCollapseWidget(
		"PLOT", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, parent);
	plotSection->contentLayout()->setSpacing(10);

	// timespan
	m_timespanSpin = new PositionSpinButton({{"ms", 1E-3}, {"s", 1E0}}, "Timespan", 0.02, 10, true, false);
	m_timespanSpin->setStep(0.05);
	m_timespanSpin->setValue(1);
	connect(m_timespanSpin, &PositionSpinButton::valueChanged, this, [=, this](double value) {
		m_voltagePlot->xAxis()->setMin(-value);
		m_currentPlot->xAxis()->setMin(-value);
	});

	// plotting mode: software trigger or rolling mode
	QWidget *plottingModeWidget = new QWidget(plotSection);
	plottingModeWidget->setLayout(new QVBoxLayout(plottingModeWidget));
	plottingModeWidget->layout()->setMargin(0);

	connect(m_runBtn, &QPushButton::toggled, plottingModeWidget, &QWidget::setDisabled);
	connect(m_singleBtn, &QPushButton::toggled, plottingModeWidget, &QWidget::setDisabled);

	m_triggeredBy = new MenuCombo("Triggered by", plottingModeWidget);
	m_triggeredBy->combo()->addItem("ua");
	m_triggeredBy->combo()->addItem("ub");
	m_triggeredBy->combo()->addItem("uc");
	m_triggeredBy->combo()->addItem("triphasic");
	connect(m_triggeredBy->combo(), &QComboBox::currentTextChanged, this,
		&WaveformInstrument::onTriggeredChnlChanged);

	MenuOnOffSwitch *rollingModeSwitch = new MenuOnOffSwitch("Rolling mode", plottingModeWidget);
	rollingModeSwitch->onOffswitch()->setChecked(false);
	connect(rollingModeSwitch->onOffswitch(), &QAbstractButton::toggled, m_triggeredBy, &MenuCombo::setDisabled);
	connect(rollingModeSwitch->onOffswitch(), &QAbstractButton::toggled, this,
		&WaveformInstrument::onRollingSwitch);
	plottingModeWidget->layout()->addWidget(m_triggeredBy);
	plottingModeWidget->layout()->addWidget(rollingModeSwitch);

	plotSection->add(m_timespanSpin);
	plotSection->add(plottingModeWidget);

	return plotSection;
}

QWidget *WaveformInstrument::createMenuLogSection(QWidget *parent)
{
	MenuSectionCollapseWidget *logSection = new MenuSectionCollapseWidget(
		"LOG", MenuCollapseSection::MHCW_ONOFF, MenuCollapseSection::MHW_BASEWIDGET, parent);
	logSection->contentLayout()->setSpacing(10);
	logSection->setCollapsed(true);

	QWidget *browseWidget = new QWidget(logSection);
	browseWidget->setLayout(new QHBoxLayout(browseWidget));
	browseWidget->layout()->setMargin(0);

	MenuLineEdit *logFilePath = new MenuLineEdit(browseWidget);
	logFilePath->edit()->setPlaceholderText("Select log directory");
	QPushButton *browseBtn = new QPushButton("...", browseWidget);
	StyleHelper::BrowseButton(browseBtn);

	browseWidget->layout()->addWidget(logFilePath);
	browseWidget->layout()->addWidget(browseBtn);

	connect(this, &WaveformInstrument::enableTool, this, [this, logFilePath, logSection](bool en) {
		logSection->setDisabled(en);
		QString dirPath = logFilePath->edit()->text();
		QDir logDir = QDir(dirPath);
		if(dirPath.isEmpty())
			logSection->setCollapsed(true);
		if(en && logDir.exists() && !logSection->collapsed())
			Q_EMIT logData(PqmDataLogger::Waveform, dirPath);
		else
			Q_EMIT logData(PqmDataLogger::None, "");
	});
	connect(this, &WaveformInstrument::enableTool, browseWidget, &QWidget::setDisabled);
	connect(browseBtn, &QPushButton::clicked, this, [this, logFilePath]() { browseFile(logFilePath->edit()); });

	logSection->add(browseWidget);

	return logSection;
}

void WaveformInstrument::stop() { m_runBtn->setChecked(false); }

void WaveformInstrument::toggleWaveform(bool en)
{
	m_running = en;
	if(en) {
		ResourceManager::open("pqm" + m_uri, this);
	} else {
		ResourceManager::close("pqm" + m_uri);
	}
	m_plottingStrategy->clearSamples();
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

void WaveformInstrument::plotData(QMap<QString, QVector<double>> chnlsData)
{
	const QStringList keys = chnlsData.keys();
	for(const QString &chnlId : keys) {
		int dataSize = chnlsData[chnlId].size();
		updateXData(dataSize);
		m_plotChnls[chnlId]->curve()->setSamples(m_xTime.data(), chnlsData[chnlId].data(), dataSize);
	}
	m_voltagePlot->replot();
	m_currentPlot->replot();
}

void WaveformInstrument::deletePlottingStrategy()
{
	if(m_plottingStrategy) {
		delete m_plottingStrategy;
		m_plottingStrategy = nullptr;
	}
}

void WaveformInstrument::createTriggeredStrategy(QString triggerChnl)
{
	triggerChnl = (triggerChnl.compare("triphasic") == 0) ? "" : triggerChnl;
	m_plottingStrategy = PlottingStrategyBuilder::build(TRIGGER_MODE, m_plotSampleRate, triggerChnl);
}

void WaveformInstrument::onTriggeredChnlChanged(QString triggeredChnl)
{
	deletePlottingStrategy();
	createTriggeredStrategy(triggeredChnl);
}

void WaveformInstrument::onRollingSwitch(bool checked)
{
	deletePlottingStrategy();
	if(checked) {
		m_plottingStrategy = PlottingStrategyBuilder::build(ROLLING_MODE, m_plotSampleRate);
	} else {
		QString triggerChnl = m_triggeredBy->combo()->currentText();
		createTriggeredStrategy(triggerChnl);
	}
}

void WaveformInstrument::onBufferDataAvailable(QMap<QString, QVector<double>> data)
{
	if(!m_running) {
		return;
	}
	int samplingFreq = m_plotSampleRate * m_timespanSpin->value();
	m_plottingStrategy->setSamplingFreq(samplingFreq);
	QMap<QString, QVector<double>> processedData = m_plottingStrategy->processSamples(data);
	if(m_plottingStrategy->dataReady()) {
		plotData(processedData);
	}
	if(m_singleBtn->isChecked() && processedData.first().size() == samplingFreq) {
		m_singleBtn->setChecked(false);
	}
}

void WaveformInstrument::browseFile(QLineEdit *lineEditPath)
{
	QString dirPath =
		QFileDialog::getExistingDirectory(this, "Select a directory", "directoryToOpen",
						  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	lineEditPath->setText(dirPath);
}

#include "moc_waveforminstrument.cpp"
