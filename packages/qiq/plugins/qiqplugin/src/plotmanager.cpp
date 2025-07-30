#include "plotmanager.h"
#include <QLoggingCategory>
#include <plotfactory.h>

Q_LOGGING_CATEGORY(CAT_PLOT_MANAGER, "PlotManager");
using namespace scopy::qiqplugin;

PlotManager::PlotManager(QObject *parent)
	: QObject(parent)
{
	m_dataManager = new DataManager(this);

	connect(m_dataManager, &DataManager::dataIsReady, this, &PlotManager::updatePlots);
	connect(m_dataManager, &DataManager::configOutput, this, &PlotManager::configOutput);
}

PlotManager::~PlotManager() {}

QList<QWidget *> PlotManager::getPlotW()
{
	QList<QWidget *> plotList;
	for(IPlot *p : qAsConst(m_plots)) {
		plotList.push_back(p->widget());
	}
	return plotList;
}

void PlotManager::samplingFreqAvailable(int samplingFreq)
{
	m_samplingFreq = samplingFreq;
	for(IPlot *p : m_plots) {
		p->setSamplingFreq(samplingFreq);
	}
}

void PlotManager::onAvailableInfo(const OutputInfo &outInfo, QList<QIQPlotInfo> plotInfoList)
{
	if(!m_plots.isEmpty()) {
		qDeleteAll(m_plots);
		m_plots.clear();
	}
	QMap<int, QList<QPair<int, int>>> plotDataMap = getPlotDataChMap(plotInfoList);
	setupDataManager(outInfo, plotDataMap);
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
	for(IPlot *plot : qAsConst(m_plots)) {
		int id = plot->id();
		QList<CurveData> curves = m_dataManager->dataForPlot(id);
		plot->updateData(curves);
	}
	Q_EMIT requestNewData();
}

QMap<int, QList<QPair<int, int>>> PlotManager::getPlotDataChMap(const QList<QIQPlotInfo> plotInfoList)
{
	QMap<int, QList<QPair<int, int>>> plotDataMap;
	for(const QIQPlotInfo &info : plotInfoList) {
		plotDataMap.insert(info.id, info.channels);
	}
	return plotDataMap;
}

void PlotManager::setupPlots(QList<QIQPlotInfo> plotInfoList)
{
	for(const QIQPlotInfo &info : qAsConst(plotInfoList)) {
		IPlot *plot = PlotFactory::createPlot(info.type);
		plot->init(info, m_samplingFreq);
		m_plots.push_back(plot);
	}
}

void PlotManager::setupDataManager(const OutputInfo &outInfo, QMap<int, QList<QPair<int, int>>> plotDataMap)
{
	int channelCount = outInfo.channelCount();
	const QStringList chnlsFormat = outInfo.channelFormat();
	m_dataManager->config(plotDataMap, chnlsFormat, channelCount);
}
