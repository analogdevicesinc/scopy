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

#include "qiqplotmanager/plotmanager.h"
#include "plotaxis.h"
#include <QLoggingCategory>
#include <stylehelper.h>

Q_LOGGING_CATEGORY(CAT_PLOT_MANAGER, "PlotManager");
using namespace scopy::qiqplugin;

PlotManager::PlotManager(QObject *parent)
	: QObject(parent)
	, m_inputPlot(nullptr)
{
	m_dataManager = new DataManager(this);
	createInputPlot();

	connect(this, &PlotManager::bufferDataReady, m_dataManager, &DataManager::onInputData);
	connect(m_dataManager, &DataManager::dataIsReady, this, &PlotManager::updatePlots);
	connect(m_dataManager, &DataManager::configOutput, this, &PlotManager::configOutput);
}

PlotManager::~PlotManager() { clearPlots(); }

QVector<QWidget *> PlotManager::getPlotW()
{
	QVector<QWidget *> widgets;
	if(m_inputPlot) {
		widgets.push_back(m_inputPlot);
	}
	for(const auto &container : qAsConst(m_plotContainers)) {
		widgets.push_back(container.widget);
	}
	return widgets;
}

void PlotManager::samplingFreqAvailable(int samplingFreq) { m_dataManager->setSamplingFreq(samplingFreq); }

void PlotManager::onAvailableInfo(const OutputInfo &outInfo, QList<QIQPlotInfo> plotInfoList)
{
	clearPlots();
	setupDataManager(outInfo);
	createPlots(plotInfoList);
}

void PlotManager::onAnalysisConfig(const QString &type, const QVariantMap &config, const OutputInfo &outInfo)
{
	m_dataManager->onConfigAnalysis(type, config, outInfo);
}

void PlotManager::onDataIsProcessed(int samplesOffset, int samplesCount)
{
	m_dataManager->readData(samplesOffset, samplesCount);
}

void PlotManager::updatePlots()
{
	updateInputData();
	for(auto &container : m_plotContainers) {
		QMap<QString, QVector<double>> data;
		const QList<QIQPlotInfo::PlotInfoCh> chnlsInfo = container.info.channels;
		for(const QIQPlotInfo::PlotInfoCh &ch : chnlsInfo) {
			data.insert(ch.x, m_dataManager->dataForKey(ch.x));
			data.insert(ch.y, m_dataManager->dataForKey(ch.y));
		}
		container.creator->updatePlot(container.widget, container.info, data);
	}
	Q_EMIT requestNewData();
}

void PlotManager::createPlots(QList<QIQPlotInfo> &plotInfoList)
{
	m_plotContainers.clear();
	for(const auto &plotInfo : plotInfoList) {
		auto creator = PlotCreatorFactory::createPlotCreator(plotInfo);
		auto widget = creator->createPlot(plotInfo);
		if(widget) {
			PlotContainer container(widget, plotInfo, creator);
			m_plotContainers.push_back(container);
		}
	}
}

void PlotManager::setupDataManager(const OutputInfo &outInfo)
{
	int channelCount = outInfo.channelCount();
	const QStringList chnlsFormat = outInfo.channelFormat();
	const QStringList chnlsName = outInfo.channelNames();
	m_dataManager->config(chnlsName, chnlsFormat, channelCount);
}

void PlotManager::clearPlots() { m_plotContainers.clear(); }

// input plot
void PlotManager::createInputPlot()
{
	m_inputPlot = new PlotWidget();
	m_inputPlot->plot()->setTitle("Input waveform");
	m_inputPlot->xAxis()->setUnits("Time");
	m_inputPlot->yAxis()->setUnits("Magnitude");

	m_inputPlot->xAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_inputPlot->xAxis()->scaleDraw()->setFloatPrecision(2);
	m_inputPlot->yAxis()->setInterval(0, 1);

	m_inputPlot->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_inputPlot->yAxis()->scaleDraw()->setFloatPrecision(2);
	m_inputPlot->yAxis()->setInterval(-200, 200);

	m_inputPlot->setShowXAxisLabels(true);
	m_inputPlot->setShowYAxisLabels(true);
	m_inputPlot->showAxisLabels();

	m_inputPlot->replot();
}

void PlotManager::updateInputPlot(int chnlCount)
{
	removePlotChannels();
	for(int i = 0; i < chnlCount; i++) {
		addPlotChannel("ch" + QString::number(i), StyleHelper::getChannelColor(i));
	}
	m_inputPlot->replot();
}

void PlotManager::removePlotChannels()
{
	const QList<PlotChannel *> chnls = m_inputPlot->getChannels();
	for(PlotChannel *ch : chnls) {
		m_inputPlot->removePlotChannel(ch);
		delete ch;
	}
	m_inputPlot->replot();
}

void PlotManager::updateInputData()
{
	int chIdx = 0;
	const QList<PlotChannel *> plotChnls = m_inputPlot->getChannels();
	for(const PlotChannel *ch : plotChnls) {
		QVector<double> xTime = m_dataManager->dataForKey(DataManagerKeys::TIME);
		QVector<double> inputData = m_dataManager->dataForKey(DataManagerKeys::INPUT + QString::number(chIdx));
		ch->curve()->setSamples(xTime, inputData);
		chIdx++;
	}
	m_inputPlot->xAxis()->setInterval(0, m_dataManager->sampleCount() / m_dataManager->samplingFreq());
	m_inputPlot->replot();
}

void PlotManager::addPlotChannel(const QString &label, const QColor &color)
{
	QPen pen(color, 1);
	PlotChannel *channel = new PlotChannel(label, pen, m_inputPlot->xAxis(), m_inputPlot->yAxis(), this);
	m_inputPlot->addPlotChannel(channel);
	channel->setEnabled(true);
}
