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

#include "qiqinstrument.h"
#include "dockablearea.h"
#include <measurementlabel.h>
#include <menucontrolbutton.h>
#include <stylehelper.h>
#include <tooltemplate.h>
#include <style.h>

using namespace scopy::qiqplugin;

QIQInstrument::QIQInstrument(ToolMenuEntry *tme, QWidget *parent)
	: QWidget(parent)
	, m_tme(tme)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);

	ToolTemplate *tool = new ToolTemplate(this);

	tool->topContainer()->setVisible(true);
	tool->topCentral()->setVisible(true);
	tool->centralContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	tool->leftContainer()->setVisible(true);
	tool->bottomContainer()->setVisible(true);

	tool->setRightContainerWidth(240);
	tool->setLeftContainerWidth(240);

	m_panel = new MeasurementsPanel(tool);
	QPushButton *measure = createMenuButton("Measure", tool);

	QPushButton *acqSettingsBtn = createMenuButton("Acquisition", tool);

	m_plotManager = new PlotManager(this);

	m_settings = new SettingsMenu(this);
	GearBtn *settingsBtn = new GearBtn(this);
	settingsBtn->setChecked(true);

	m_runBtn = new RunBtn(this);
	m_runBtn->setDisabled(true);

	m_singleBtn = new SingleShotBtn(this);
	m_singleBtn->setDisabled(true);

	m_dockableArea = createDockableArea(this);
	QWidget *dockableAreaWidget = dynamic_cast<QWidget *>(m_dockableArea);
	Style::setBackgroundColor(dockableAreaWidget, json::theme::background_subtle, true);

	tool->addWidgetToCentralContainerHelper(dockableAreaWidget);
	tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(settingsBtn, TTA_RIGHT);
	tool->addWidgetToBottomContainerHelper(acqSettingsBtn, TTA_RIGHT);
	tool->addWidgetToBottomContainerHelper(measure, TTA_RIGHT);

	tool->topStack()->add(MEASURE_PANEL_ID, m_panel);
	tool->rightStack()->add("settings", m_settings->plotW());
	tool->leftStack()->add("acqSettings", m_settings->acqW());

	layout->addWidget(tool);

	setupConnections();
	connect(measure, &QPushButton::toggled, this, [tool](bool b) { tool->openTopContainerHelper(b); });
	connect(settingsBtn, &QPushButton::toggled, this, [tool](bool b) { tool->openRightContainerHelper(b); });
	connect(acqSettingsBtn, &QPushButton::toggled, this, [tool](bool b) { tool->openLeftContainerHelper(b); });
}

QIQInstrument::~QIQInstrument() {}

void QIQInstrument::setupConnections()
{
	connect(m_runBtn, &RunBtn::toggled, this, &QIQInstrument::runPressed);
	connect(m_runBtn, &RunBtn::toggled, m_settings->acqW(), &QWidget::setDisabled);
	connect(m_runBtn, &RunBtn::toggled, m_tme, &ToolMenuEntry::setRunning);
	connect(m_runBtn, &RunBtn::toggled, m_singleBtn, &SingleShotBtn::setDisabled);
	connect(m_singleBtn, &SingleShotBtn::toggled, this, &QIQInstrument::runPressed);
	connect(m_singleBtn, &SingleShotBtn::toggled, m_runBtn, &RunBtn::setDisabled);
	connect(m_singleBtn, &SingleShotBtn::toggled, m_settings->acqW(), &QWidget::setDisabled);
	connect(m_tme, &ToolMenuEntry::runClicked, this, &QIQInstrument::tmeToggled);
	connect(m_settings, &SettingsMenu::analysisChanged, this, &QIQInstrument::requestAnalysisInfo);
	connect(m_settings, &SettingsMenu::analysisConfig, this, &QIQInstrument::analysisConfigChanged);
	connect(m_settings, &SettingsMenu::bufferParamsChanged, this, &QIQInstrument::bufferParamsChanged);
	connect(m_settings, &SettingsMenu::plotSettings, m_plotManager, &PlotManager::plotSettingsRequest);
	connect(m_plotManager, &PlotManager::plotSettings, m_settings, &SettingsMenu::onSettingsMenu);
	connect(this, &QIQInstrument::bufferDataReady, m_plotManager, &PlotManager::bufferDataReady);
	connect(m_plotManager, &PlotManager::requestNewData, this, &QIQInstrument::requestNewData);
}

void QIQInstrument::setAvailableChannels(QMap<QString, QList<ChannelInfo>> channels)
{
	m_settings->setAvailableChannels(channels);
}

void QIQInstrument::onAnalysisTypes(const QStringList &types) { m_settings->setAnalysisTypes(types); }

void QIQInstrument::onInputFormatChanged(const InputConfig &inConfig)
{
	m_plotManager->samplingFreqAvailable(inConfig.samplingFrequency());
	m_plotManager->updateInputPlot(inConfig.channelCount());
	m_inputFormatConfigured = inConfig.channelCount() > 0;
	enableAcquisition();
	// must be deleted
	configureOutput();
}

void QIQInstrument::onOutputConfig(const OutputConfig &outConfig)
{
	m_outputConfigured = true;
	enableAcquisition();
}

void QIQInstrument::onRunResponse(const RunResults &runResults)
{
	QVariantMap resultsMap = runResults.getResults();
	int offset = resultsMap.value("offset", 0).toInt();
	int samples = resultsMap.value("samples_size", 0).toInt();
	m_plotManager->onDataIsProcessed(offset, samples);
	updateMeasurements(runResults.getMeasurements());
	if(m_singleBtn->isChecked()) {
		m_singleBtn->setChecked(false);
	}
}

void QIQInstrument::onAnalysisInfo(const QString &type, const QVariantMap &params, const OutputInfo &outputInfo,
				   const QList<QIQPlotInfo> plotInfoList, QStringList measurements)
{
	m_plotManager->onAvailableInfo(outputInfo, plotInfoList);
	fillMeasurementsPanel(measurements);
	m_settings->setAnalysisParams(type, params);
	m_settings->setPlotTitle(m_plotManager->plotTitle());
	addPlots();
}

void QIQInstrument::onAnalysisConfigured(const QString &type, const QVariantMap &config, const OutputInfo &outputInfo)
{
	m_settings->validateAnalysisParams(type, config);
	m_plotManager->onAnalysisConfig(type, config, outputInfo);
	configureOutput();
}

void QIQInstrument::tmeToggled(bool checked)
{
	if(!m_runBtn->isEnabled()) {
		m_tme->setRunning(false);
		return;
	}
	m_runBtn->setChecked(checked);
}

void QIQInstrument::onProcessFinished(int exitCode)
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

void QIQInstrument::addPlots()
{
	const QVector<DockWrapperInterface *> dockList = m_plotManager->plotWrappers();
	for(DockWrapperInterface *w : dockList) {
		m_dockableArea->addDockWrapper(w, DockableAreaInterface::Direction_BOTTOM);
	}
}

void QIQInstrument::clearMeasurementLabels()
{
	if(m_labels.isEmpty()) {
		return;
	}
	m_labels.clear();
	m_panel->clear();
}

void QIQInstrument::configureOutput()
{
	QString type = m_settings->getCrtAnalysisType();
	OutputConfig outConfig;
	outConfig.setOutputFile(QIQUtils::dataOutPath());
	outConfig.setOutputFileFormat(FileFormatTypes::BINARY_INTERLEAVED);
	outConfig.setEnabledAnalysis({type});

	Q_EMIT outputConfigured(outConfig);
}

void QIQInstrument::enableAcquisition()
{
	bool enAcq = m_outputConfigured && m_inputFormatConfigured;
	m_runBtn->setEnabled(enAcq);
	m_singleBtn->setEnabled(enAcq);
}

void QIQInstrument::updateMeasurements(const QVariantMap &measurements)
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

void QIQInstrument::fillMeasurementsPanel(const QStringList &measurements)
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

QPushButton *QIQInstrument::createMenuButton(const QString &name, QWidget *parent)
{
	QPushButton *btn = new QPushButton(name, parent);
	Style::setStyle(btn, style::properties::button::blueGrayButton);
	btn->setCheckable(true);
	btn->setChecked(true);
	return btn;
}
