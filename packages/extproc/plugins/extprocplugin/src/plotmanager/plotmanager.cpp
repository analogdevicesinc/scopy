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

#include "plotmanager/plotmanager.h"
#include "extprocutils.h"
#include "dockwrapper.h"
#include <QLoggingCategory>
#include <controller/extprocplotinfo.h>

Q_LOGGING_CATEGORY(CAT_PLOT_MANAGER, "PlotManager");
using namespace scopy::extprocplugin;

PlotManager::PlotManager(QObject *parent)
	: QObject(parent)
{
	m_dataManager = DataManager::GetInstance();
	connect(m_dataManager, &DataManager::newDataEntries, this, &PlotManager::dataManagerEntries);
}

PlotManager::~PlotManager()
{
	// Temporary solution — not the proper approach for an app-focused data manager.
	m_dataManager->clearData();
}

void PlotManager::samplingFreqAvailable(int samplingFreq) { m_dataManager->setSamplingFreq(samplingFreq); }

void PlotManager::onAvailableInfo(const OutputInfo &outInfo, QList<ExtProcPlotInfo> plotInfoList)
{
	clearPlots();
	createPlots(plotInfoList);
}

void PlotManager::plotSettingsRequest(const QString &plot)
{
	QWidget *w = nullptr;
	for(const PlotContainer &c : qAsConst(m_plotContainers)) {
		if(c.creator->plotInfo().title == plot) {
			w = c.creator->settingsMenu();
			break;
		}
	}
	Q_EMIT plotSettings(w);
}

void PlotManager::updatePlots()
{
	for(auto &container : m_plotContainers) {
		QMap<QString, QVector<float>> data;
		const QList<ExtProcPlotInfo::PlotInfoCh> chnlsInfo = container.creator->plotInfo().channels;
		for(const ExtProcPlotInfo::PlotInfoCh &ch : chnlsInfo) {
			data.insert(ch.x, m_dataManager->dataForKey(ch.x));
			data.insert(ch.y, m_dataManager->dataForKey(ch.y));
		}
		container.creator->updatePlot(data);
	}
	Q_EMIT requestNewData();
}

void PlotManager::createPlots(QList<ExtProcPlotInfo> &plotInfoList)
{
	QSet<QString> dmEntries;
	QList<ExtProcPlotInfo> list = {inputPlot()};
	list.append(plotInfoList);
	m_plotContainers.clear();
	for(const auto &plotInfo : list) {
		for(const ExtProcPlotInfo::PlotInfoCh &ch : plotInfo.channels) {
			dmEntries.insert(ch.x);
			dmEntries.insert(ch.y);
		}
		auto creator = PlotCreatorFactory::createPlotCreator(plotInfo, this);
		DockWrapperInterface *plotWrapper = createDockWrapper(plotInfo.title);
		plotWrapper->setInnerWidget(creator->plot());
		PlotContainer container{plotWrapper, creator};
		m_plotContainers.insert(plotInfo.id, container);
		connect(creator, SIGNAL(requestSettings(QString)), this, SIGNAL(changeSettings(QString)));
		connect(this, SIGNAL(dataManagerEntries(QStringList)), creator,
			SIGNAL(dataManagerEntries(QStringList)));
	}
	m_plotContainers.value(INPUT_PLOT_ID).creator->enableChannelAdd(false);
	Q_EMIT dataManagerEntries(dmEntries.values());
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
	QList<ExtProcPlotInfo::PlotInfoCh> inputChannels;
	for(int i = 0; i < chnlCount; i++) {
		inputChannels.append({DataManagerKeys::TIME, DataManagerKeys::INPUT + QString::number(i)});
	}
	PlotCreatorBase *inputCreator = m_plotContainers[INPUT_PLOT_ID].creator;
	ExtProcPlotInfo plotInfo = m_plotContainers[INPUT_PLOT_ID].creator->plotInfo();
	plotInfo.channels = inputChannels;
	inputCreator->setPlotInfo(plotInfo);
}

ExtProcPlotInfo PlotManager::inputPlot()
{
	ExtProcPlotInfo info;
	info.id = INPUT_PLOT_ID;
	info.title = INPUT_PLOT_TITLE;
	info.xLabel = "time[s]";
	info.yLabel = "amplitude[V]";
	info.type = ExtProcPlotInfo::PLOT_WIDGET;
	info.flags = QStringList("labels");
	info.channels = {{DataManagerKeys::TIME, DataManagerKeys::INPUT + QString::number(0)}};
	return info;
}

QStringList PlotManager::plotTitle() const
{
	QStringList titleList;
	for(const PlotContainer &c : m_plotContainers) {
		titleList.push_back(c.creator->plotInfo().title);
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
