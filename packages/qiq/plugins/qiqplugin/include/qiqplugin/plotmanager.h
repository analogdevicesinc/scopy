#ifndef PLOTMANAGER_H
#define PLOTMANAGER_H

#include "outputinfo.h"
#include <QObject>
#include <datamanager.h>
#include <iplot.h>
#include <gui/plotwidget.h>

namespace scopy::qiqplugin {

class PlotManager : public QObject
{
	Q_OBJECT
public:
	PlotManager(QObject *parent = nullptr);
	~PlotManager();

	QList<QWidget *> getPlotW();

Q_SIGNALS:
	void requestNewData();

public Q_SLOTS:
	void samplingFreqAvailable(int samplingFreq);
	void onAvailableInfo(const OutputInfo &outInfo, QList<QIQPlotInfo> plotInfoList);
	void onDataIsProcessed(int samplesOffset, int samplesCount);
	void updatePlots();

private:
	QMap<int, QList<QPair<int, int>>> getPlotDataChMap(const QList<QIQPlotInfo> plotInfoList);
	void setupPlots(QList<QIQPlotInfo> plotInfoList);
	void setupDataManager(const OutputInfo &outInfo, QMap<int, QList<QPair<int, int>>> chnlsMap);

	int m_samplingFreq = 512;
	DataManager *m_dataManager;
	QList<IPlot *> m_plots;
};

} // namespace scopy::qiqplugin

#endif // PLOTMANAGER_H
