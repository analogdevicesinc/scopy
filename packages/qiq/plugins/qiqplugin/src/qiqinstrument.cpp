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
#include "dockwrapper.h"
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
	tool->bottomContainer()->setVisible(true);

	tool->setRightContainerWidth(280);

	m_panel = new MeasurementsPanel(tool);
	QPushButton *measure = new QPushButton("Measure", tool);
	Style::setStyle(measure, style::properties::button::blueGrayButton);
	measure->setCheckable(true);
	measure->setChecked(true);

	m_plotManager = new PlotManager(this);

	m_settings = new SettingsMenu(this);
	GearBtn *settingsBtn = new GearBtn(this);
	settingsBtn->setChecked(true);

	m_runBtn = new RunBtn(this);
	m_runBtn->setDisabled(true);

	m_dockableArea = createDockableArea(this);
	QWidget *dockableAreaWidget = dynamic_cast<QWidget *>(m_dockableArea);
	Style::setBackgroundColor(dockableAreaWidget, json::theme::background_subtle, true);
	addInputPlot();

	tool->addWidgetToCentralContainerHelper(dockableAreaWidget);
	tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(settingsBtn, TTA_RIGHT);
	tool->addWidgetToBottomContainerHelper(measure, TTA_RIGHT);

	tool->topStack()->add(MEASURE_PANEL_ID, m_panel);
	tool->rightStack()->add("settings", m_settings);

	layout->addWidget(tool);

	setupConnections();
	connect(measure, &QPushButton::toggled, this, [tool](bool b) { tool->openTopContainerHelper(b); });
	connect(m_runBtn, &RunBtn::toggled, this, &QIQInstrument::runPressed);
	connect(m_runBtn, &RunBtn::toggled, m_settings, &SettingsMenu::setDisabled);
	connect(m_runBtn, &RunBtn::toggled, tme, &ToolMenuEntry::setRunning);
	connect(m_tme, &ToolMenuEntry::runClicked, this, &QIQInstrument::tmeToggled);
	connect(settingsBtn, &QPushButton::toggled, this, [=, this](bool b) { tool->openRightContainerHelper(b); });
}

QIQInstrument::~QIQInstrument() {}

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
	m_runBtn->setEnabled(m_outputConfigured && m_inputFormatConfigured);
}

void QIQInstrument::onOutputConfig(const OutputConfig &outConfig) {}

void QIQInstrument::onRunResponse(const RunResults &runResults)
{
	QVariantMap resultsMap = runResults.getResults();
	int offset = resultsMap.value("offset", 0).toInt();
	int samples = resultsMap.value("samples_size", 0).toInt();
	m_plotManager->onDataIsProcessed(offset, samples);
	updateMeasurements(runResults.getMeasurements());
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
		m_runBtn->setEnabled(false);
		m_inputFormatConfigured = false;
		m_outputConfigured = false;
	}
}

void QIQInstrument::addPlots()
{
	const QVector<DockWrapperInterface *> dockList = m_plotManager->plotWrappers();
	for(DockWrapperInterface *w : dockList) {
		m_dockableArea->addDockWrapper(w, DockableAreaInterface::Direction_BOTTOM);
	}
}

void QIQInstrument::addInputPlot()
{
	DockWrapperInterface *plotWrapper = createDockWrapper(INPUT_PLOT_TITLE);
	plotWrapper->setInnerWidget(m_plotManager->inputPlot());
	m_dockableArea->addDockWrapper(plotWrapper, DockableAreaInterface::Direction_TOP);
}

void QIQInstrument::setupConnections()
{
	connect(m_settings, &SettingsMenu::analysisChanged, this, &QIQInstrument::requestAnalysisInfo);
	connect(m_settings, &SettingsMenu::analysisConfig, this, &QIQInstrument::analysisConfigChanged);
	connect(m_settings, &SettingsMenu::bufferParamsChanged, this, &QIQInstrument::bufferParamsChanged);
	connect(m_settings, &SettingsMenu::plotSettings, m_plotManager, &PlotManager::plotSettingsRequest);
	connect(m_plotManager, &PlotManager::plotSettings, m_settings, &SettingsMenu::onSettingsMenu);
	connect(this, &QIQInstrument::bufferDataReady, m_plotManager, &PlotManager::bufferDataReady);
	connect(m_plotManager, &PlotManager::requestNewData, this, &QIQInstrument::requestNewData);
	connect(m_plotManager, &PlotManager::configOutput, this, &QIQInstrument::outputConfigured);
	connect(m_plotManager, &PlotManager::configOutput, this, [this]() {
		m_outputConfigured = true;
		m_runBtn->setEnabled(m_outputConfigured && m_inputFormatConfigured);
	});
}

void QIQInstrument::clearMeasurementLabels()
{
	if(m_labels.isEmpty()) {
		return;
	}
	m_labels.clear();
	m_panel->clear();
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
