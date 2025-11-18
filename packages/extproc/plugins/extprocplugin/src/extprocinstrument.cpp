/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "extprocinstrument.h"
#include "dockablearea.h"
#include "dockwrapper.h"
#include "extprocutils.h"
#include <measurementlabel.h>
#include <menucontrolbutton.h>
#include <stylehelper.h>
#include <tooltemplate.h>
#include <style.h>

using namespace scopy::extprocplugin;

ExtProcInstrument::ExtProcInstrument(ToolMenuEntry *tme, QWidget *parent)
	: QWidget(parent)
	, m_tme(tme)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);

	ToolTemplate *tool = new ToolTemplate(this);

	tool->topContainer()->setVisible(true);
	tool->centralContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	tool->leftContainer()->setVisible(true);
	tool->bottomContainer()->setVisible(true);

	tool->setRightContainerWidth(300);
	tool->setLeftContainerWidth(240);

	m_dockableArea = createDockableArea(this);
	QWidget *dockableAreaWidget = dynamic_cast<QWidget *>(m_dockableArea);
	Style::setBackgroundColor(dockableAreaWidget, json::theme::background_subtle, true);

	DockWrapperInterface *panelWrapper = createDockWrapper("Measurements");
	m_panel = new MeasurementsPanel(dockableAreaWidget);
	m_panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	panelWrapper->setInnerWidget(m_panel);
	QPushButton *measure = createMenuButton("Measure", tool);
	m_dockableArea->addDockWrapper(panelWrapper);

	QPushButton *acqSettingsBtn = createMenuButton("Acquisition", tool);

	m_plotManager = new PlotManager(this);

	m_dataReader = new DataReader(this);
	m_dataReader->openFile(ExtProcUtils::dataOutPath());

	m_dataProcessingService = new DataProcessingService(this);

	m_settings = new SettingsMenu(this);
	GearBtn *settingsBtn = new GearBtn(this);
	settingsBtn->setChecked(true);

	m_runBtn = new RunBtn(this);
	m_runBtn->setDisabled(true);

	m_singleBtn = new SingleShotBtn(this);
	m_singleBtn->setDisabled(true);

	tool->addWidgetToCentralContainerHelper(dockableAreaWidget);
	tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(settingsBtn, TTA_RIGHT);
	tool->addWidgetToBottomContainerHelper(acqSettingsBtn, TTA_RIGHT);
	tool->addWidgetToBottomContainerHelper(measure, TTA_RIGHT);

	tool->rightStack()->add("settings", m_settings->plotW());
	tool->leftStack()->add("acqSettings", m_settings->acqW());

	layout->addWidget(tool);

	setupConnections();
	connect(measure, &QPushButton::toggled, this, [panelWrapper](bool b) { panelWrapper->setActivated(b); });
	connect(settingsBtn, &QPushButton::toggled, this, [tool](bool b) { tool->openRightContainerHelper(b); });
	connect(acqSettingsBtn, &QPushButton::toggled, this, [tool](bool b) { tool->openLeftContainerHelper(b); });
}

ExtProcInstrument::~ExtProcInstrument() {}

void ExtProcInstrument::setupConnections()
{
	connect(m_runBtn, &RunBtn::toggled, this, &ExtProcInstrument::runPressed);
	connect(m_runBtn, &RunBtn::toggled, m_settings->acqW(), &QWidget::setDisabled);
	connect(m_runBtn, &RunBtn::toggled, m_tme, &ToolMenuEntry::setRunning);
	connect(m_runBtn, &RunBtn::toggled, m_singleBtn, &SingleShotBtn::setDisabled);
	connect(m_singleBtn, &SingleShotBtn::toggled, this, &ExtProcInstrument::runPressed);
	connect(m_singleBtn, &SingleShotBtn::toggled, m_runBtn, &RunBtn::setDisabled);
	connect(m_singleBtn, &SingleShotBtn::toggled, m_settings->acqW(), &QWidget::setDisabled);
	connect(m_tme, &ToolMenuEntry::runClicked, this, &ExtProcInstrument::tmeToggled);
	connect(m_settings, &SettingsMenu::analysisChanged, this, &ExtProcInstrument::requestAnalysisInfo);
	connect(m_settings, &SettingsMenu::analysisConfig, this, &ExtProcInstrument::analysisConfigChanged);
	connect(m_settings, &SettingsMenu::bufferParamsChanged, this, &ExtProcInstrument::bufferParamsChanged);
	connect(m_settings, &SettingsMenu::bufferParamsChanged, this, &ExtProcInstrument::verifyBufferParams);
	connect(m_settings, &SettingsMenu::plotSettings, m_plotManager, &PlotManager::plotSettingsRequest);
	connect(m_settings, &SettingsMenu::fftEnabled, m_dataProcessingService, &DataProcessingService::setFFTEnabled);
	connect(m_plotManager, &PlotManager::plotSettings, m_settings, &SettingsMenu::onSettingsMenu);
	connect(m_plotManager, &PlotManager::requestNewData, this, &ExtProcInstrument::requestNewData);
	connect(m_plotManager, &PlotManager::changeSettings, m_settings, &SettingsMenu::changeSettings);

	// Connect DataReader to DataManager singleton and trigger plot updates
	connect(m_dataReader, &DataReader::dataReady, this, [this](QMap<QString, QVector<float>> &data) {
		DataManager::GetInstance()->registerData(data);
		m_plotManager->updatePlots();
	});
}

void ExtProcInstrument::setAvailableChannels(QMap<QString, QList<ChannelInfo>> channels)
{
	m_settings->setAvailableChannels(channels);
}

void ExtProcInstrument::onAnalysisTypes(const QStringList &types) { m_settings->setAnalysisTypes(types); }

void ExtProcInstrument::onInputFormatChanged(const InputConfig &inConfig)
{
	m_plotManager->samplingFreqAvailable(inConfig.samplingFrequency());
	m_dataProcessingService->setSamplingFreq(inConfig.samplingFrequency());
	m_plotManager->updateInputPlot(inConfig.channelCount());
	m_inputFormatConfigured = inConfig.channelCount() > 0;
	enableAcquisition();
	// must be deleted
	configureOutput();
}

void ExtProcInstrument::onOutputConfig(const OutputConfig &outConfig)
{
	m_outputConfigured = true;
	enableAcquisition();
}

void ExtProcInstrument::onRunResponse(const RunResults &runResults)
{
	QVariantMap resultsMap = runResults.getResults();
	int offset = resultsMap.value("offset", 0).toInt();
	int samples = resultsMap.value("samples_size", 0).toInt();
	m_dataReader->readData(offset, samples);
	updateMeasurements(runResults.getMeasurements());
	if(m_singleBtn->isChecked()) {
		m_singleBtn->setChecked(false);
	}
}

void ExtProcInstrument::onAnalysisInfo(const QString &type, const QVariantMap &params, const OutputInfo &outputInfo,
				       const QList<ExtProcPlotInfo> plotInfoList, QStringList measurements)
{
	m_plotManager->onAvailableInfo(outputInfo, plotInfoList);
	setupDataReader(outputInfo);
	fillMeasurementsPanel(measurements);
	m_settings->setAnalysisParams(type, params);
	m_settings->setPlotTitle(m_plotManager->plotTitle());
	addPlots();
}

void ExtProcInstrument::onAnalysisConfigured(const QString &type, const QVariantMap &config,
					     const OutputInfo &outputInfo)
{
	m_settings->validateAnalysisParams(type, config);
	configureOutput();
}

void ExtProcInstrument::tmeToggled(bool checked)
{
	if(!m_runBtn->isEnabled()) {
		m_tme->setRunning(false);
		return;
	}
	m_runBtn->setChecked(checked);
}

void ExtProcInstrument::onProcessFinished(int exitCode)
{
	if(m_runBtn->isChecked()) {
		m_runBtn->setChecked(false);
	}
	if(m_singleBtn->isChecked()) {
		m_runBtn->setChecked(false);
	}
	m_inputFormatConfigured = false;
	m_outputConfigured = false;
	enableAcquisition();
}

void ExtProcInstrument::onBufferDataReady(QVector<QVector<float>> &inputData)
{
	if(inputData.isEmpty()) {
		return;
	}
	for(int chIdx = 0; chIdx < inputData.size(); chIdx++) {
		QString inName = DataManagerKeys::INPUT + QString::number(chIdx);
		DataManager::GetInstance()->registerData(inName, inputData[chIdx]);
	}
	m_dataProcessingService->processBufferData(inputData);
}

void ExtProcInstrument::addPlots()
{
	const QVector<DockWrapperInterface *> dockList = m_plotManager->plotWrappers();
	for(DockWrapperInterface *w : dockList) {
		m_dockableArea->addDockWrapper(w, DockableAreaInterface::Direction_BOTTOM);
	}
}

void ExtProcInstrument::clearMeasurementLabels()
{
	if(m_labels.isEmpty()) {
		return;
	}
	m_labels.clear();
	m_panel->clear();
}

void ExtProcInstrument::configureOutput()
{
	QString type = m_settings->getCrtAnalysisType();
	OutputConfig outConfig;
	outConfig.setOutputFile(ExtProcUtils::dataOutPath());
	outConfig.setOutputFileFormat(FileFormatTypes::BINARY_INTERLEAVED);
	outConfig.setEnabledAnalysis({type});

	Q_EMIT outputConfigured(outConfig);
}

void ExtProcInstrument::enableAcquisition()
{
	bool enAcq = m_outputConfigured && m_inputFormatConfigured;
	m_runBtn->setEnabled(enAcq);
	m_singleBtn->setEnabled(enAcq);
}

void ExtProcInstrument::updateMeasurements(const QVariantMap &measurements)
{
	for(auto it = measurements.cbegin(); it != measurements.cend(); ++it) {
		const QString type = it.key();
		if(!m_labels.contains(type)) {
			return;
		}
		QVariantMap typeMap = it.value().toMap();
		m_labels[type]->setValue(typeMap.value("value", 0).toDouble());
		m_labels[type]->setUnit(typeMap.value("units", "").toString());
	}
}

void ExtProcInstrument::fillMeasurementsPanel(const QStringList &measurements)
{
	clearMeasurementLabels();
	for(const QString &l : measurements) {
		MeasurementLabel *ml = new MeasurementLabel(m_panel);
		ml->setPrecision(6);
		ml->setName(l);
		m_labels.insert(l, ml);
		m_panel->addMeasurement(ml);
	}
}

void ExtProcInstrument::setupDataReader(const OutputInfo &outInfo)
{
	int channelCount = outInfo.channelCount();
	const QStringList chnlsFormat = outInfo.channelFormat();
	const QStringList chnlsName = outInfo.channelNames();

	m_dataReader->setChannelCount(channelCount);
	m_dataReader->setChannelFormat(chnlsFormat);
	m_dataReader->setChannelsName(chnlsName);
}

void ExtProcInstrument::verifyBufferParams(const BufferParams &params)
{
	if(params.enChnls.isEmpty()) {
		m_runBtn->setDisabled(true);
		m_singleBtn->setDisabled(true);
	}
}

QPushButton *ExtProcInstrument::createMenuButton(const QString &name, QWidget *parent)
{
	QPushButton *btn = new QPushButton(name, parent);
	Style::setStyle(btn, style::properties::button::blueGrayButton);
	btn->setCheckable(true);
	btn->setChecked(true);
	return btn;
}
