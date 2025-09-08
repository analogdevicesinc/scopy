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
#include "dockwrapper.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_PLOT_MANAGER, "PlotManager");
using namespace scopy::qiqplugin;

PlotManager::PlotManager(QObject *parent)
	: QObject(parent)
{
	m_dataManager = new DataManager(this);
	connect(this, &PlotManager::bufferDataReady, m_dataManager, &DataManager::onInputData);
	connect(m_dataManager, &DataManager::dataIsReady, this, &PlotManager::updatePlots);
}

PlotManager::~PlotManager() {}

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

void PlotManager::plotSettingsRequest(const QString &plot)
{
	QWidget *w = nullptr;
	for(const PlotContainer &c : qAsConst(m_plotContainers)) {
		if(c.info.title == plot) {
			w = c.creator->settingsMenu();
			break;
		}
	}
	Q_EMIT plotSettings(w);
}

void PlotManager::onDataIsProcessed(int samplesOffset, int samplesCount)
{
	m_dataManager->readData(samplesOffset, samplesCount);
}

void PlotManager::updatePlots()
{
	for(auto &container : m_plotContainers) {
		QMap<QString, QVector<double>> data;
		const QList<QIQPlotInfo::PlotInfoCh> chnlsInfo = container.info.channels;
		for(const QIQPlotInfo::PlotInfoCh &ch : chnlsInfo) {
			data.insert(ch.x, m_dataManager->dataForKey(ch.x));
			data.insert(ch.y, m_dataManager->dataForKey(ch.y));
		}
		container.creator->updatePlot(container.plot, container.info, data);
	}
	Q_EMIT requestNewData();
}

void PlotManager::createPlots(QList<QIQPlotInfo> &plotInfoList)
{
	QList<QIQPlotInfo> list = {inputPlot()};
	list.append(plotInfoList);
	m_plotContainers.clear();
	for(const auto &plotInfo : list) {
		auto creator = PlotCreatorFactory::createPlotCreator(plotInfo);
		auto plot = creator->createPlot(plotInfo);
		if(!plot) {
			continue;
		}
		DockWrapperInterface *plotWrapper = createDockWrapper(plotInfo.title);
		plotWrapper->setInnerWidget(plot);
		PlotContainer container(plot, plotWrapper, plotInfo, creator);
		m_plotContainers.insert(plotInfo.id, container);
	}
}

void PlotManager::setupDataManager(const OutputInfo &outInfo)
{
	int channelCount = outInfo.channelCount();
	const QStringList chnlsFormat = outInfo.channelFormat();
	const QStringList chnlsName = outInfo.channelNames();
	m_dataManager->config(chnlsName, chnlsFormat, channelCount);
}

void PlotManager::clearPlots()
{
	for(const PlotContainer &c : qAsConst(m_plotContainers)) {
		delete c.creator;
		QWidget *w = dynamic_cast<QWidget *>(c.plotWrapper);
		if(w) {
			delete w;
		}
	}
	m_plotContainers.clear();
}

void PlotManager::updateInputPlot(int chnlCount)
{
	QList<QIQPlotInfo::PlotInfoCh> inputChannels;
	for(int i = 0; i < chnlCount; i++) {
		inputChannels.append({"time", DataManagerKeys::INPUT + QString::number(i)});
	}
	m_plotContainers[INPUT_PLOT_ID].info.channels = inputChannels;
}

QIQPlotInfo PlotManager::inputPlot()
{
	QIQPlotInfo info;
	info.id = INPUT_PLOT_ID;
	info.title = INPUT_PLOT_TITLE;
	info.xLabel = "time[s]";
	info.yLabel = "amplitude[V]";
	info.type = QIQPlotInfo::PLOT_WIDGET;
	info.flags = QStringList("labels");
	info.channels = {{"time", DataManagerKeys::INPUT + QString::number(0)}};
	return info;
}

QStringList PlotManager::plotTitle() const
{
	QStringList titleList;
	titleList.push_back(INPUT_PLOT_TITLE);
	for(const PlotContainer &c : m_plotContainers) {
		titleList.push_back(c.info.title);
	}
	return titleList;
}

QVector<scopy::DockWrapperInterface *> PlotManager::plotWrappers() const
{
	QVector<DockWrapperInterface *> wrappers;
	for(const PlotContainer &c : m_plotContainers) {
		wrappers.push_back(c.plotWrapper);
	}
	return wrappers;
}
