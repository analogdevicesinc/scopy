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

#include "plotmanager.h"
#include "plotaxis.h"
#include <QLoggingCategory>
#include <plotfactory.h>
#include <stylehelper.h>

Q_LOGGING_CATEGORY(CAT_PLOT_MANAGER, "PlotManager");
using namespace scopy::qiqplugin;

PlotManager::PlotManager(QObject *parent)
	: QObject(parent)
{
	m_dataManager = new DataManager(this);

	connect(this, &PlotManager::bufferDataReady, m_dataManager, &DataManager::onInputData);
	connect(m_dataManager, &DataManager::dataIsReady, this, &PlotManager::updatePlots);
	connect(m_dataManager, &DataManager::configOutput, this, &PlotManager::configOutput);
}

PlotManager::~PlotManager() {}

QVector<QWidget *> PlotManager::getPlotW()
{
	QVector<QWidget *> plots;
	for(PlotWidget *plot : m_plots) {
		plots.push_back(plot);
	}
	return plots;
}

void PlotManager::samplingFreqAvailable(int samplingFreq)
{
	m_samplingFreq = samplingFreq;
	m_dataManager->setSamplingFreq(samplingFreq);
}

void PlotManager::onAvailableInfo(const OutputInfo &outInfo, QList<QIQPlotInfo> plotInfoList)
{
	if(!m_plots.isEmpty()) {
		qDeleteAll(m_plots);
		m_plots.clear();
	}
	setupDataManager(outInfo);
	setupPlots(plotInfoList);
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
	for(int plotIdx = 0; plotIdx < m_plots.size(); plotIdx++) {
		const QList<QIQPlotInfo::PlotInfoCh> channels = m_plotsInfo[plotIdx].channels;
		QList<PlotChannel *> plotChnls = m_plots[plotIdx]->getChannels();
		int chIdx = 0;
		double xFirst = 0, xLast = 0;
		for(const QIQPlotInfo::PlotInfoCh &ch : channels) {
			QVector<double> xData = m_dataManager->dataForKey(ch.x);
			QVector<double> yData = m_dataManager->dataForKey(ch.y);
			plotChnls[chIdx]->curve()->setSamples(xData, yData);
			chIdx++;
			if(xFirst == 0 && xLast == 0) {
				xFirst = xData.first();
				xLast = xData.last();
			}
		}
		if(!m_plotsInfo[plotIdx].flags.contains("points")) {
			m_plots[plotIdx]->xAxis()->setInterval(xFirst, xLast);
		}
		m_plots[plotIdx]->replot();
	}

	Q_EMIT requestNewData();
}

scopy::PlotWidget *PlotManager::createPlotWidget(QIQPlotInfo plotInfo)
{
	PlotWidget *plot = new PlotWidget();
	bool showLabels = plotInfo.flags.contains("labels");
	plot->plot()->setTitle(plotInfo.title);
	int i = 0;
	for(QIQPlotInfo::PlotInfoCh plotCh : plotInfo.channels) {
		QString chId = "ch" + QString::number(i);
		QPen pen(StyleHelper::getChannelColor(i));
		PlotChannel *ch = new PlotChannel(chId, pen, plot->xAxis(), plot->yAxis(), plot);
		plot->addPlotChannel(ch);
		ch->setEnabled(true);
		i++;
		if(plotInfo.flags.contains("points")) {
			ch->setThickness(3);
			ch->setStyle(PlotChannel::PlotCurveStyle::PCS_DOTS);
		}
	}

	plot->xAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	plot->xAxis()->scaleDraw()->setFloatPrecision(2);
	if(plotInfo.flags.contains("points")) {
		plot->xAxis()->setInterval(-200, 200);
	}

	plot->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	plot->yAxis()->scaleDraw()->setFloatPrecision(2);
	plot->yAxis()->setInterval(-200, 200);

	plot->setShowXAxisLabels(true);
	plot->setShowYAxisLabels(true);
	plot->showAxisLabels();
	plot->replot();

	return plot;
}

void PlotManager::setupPlots(QList<QIQPlotInfo> plotInfoList)
{
	m_plots.clear();
	m_plotsInfo.clear();
	for(const QIQPlotInfo &info : qAsConst(plotInfoList)) {
		PlotWidget *plot = createPlotWidget(info);
		m_plots.push_back(plot);
		m_plotsInfo.push_back(info);
	}
}

void PlotManager::setupDataManager(const OutputInfo &outInfo)
{
	int channelCount = outInfo.channelCount();
	const QStringList chnlsFormat = outInfo.channelFormat();
	const QStringList chnlsName = outInfo.channelNames();
	m_dataManager->config(chnlsName, chnlsFormat, channelCount);
}
