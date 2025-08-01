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

	QVector<QWidget *> getPlotW();

Q_SIGNALS:
	void configOutput(const OutputConfig &outConfig);
	void requestNewData();
	void bufferDataReady(QVector<QVector<double>> data);

public Q_SLOTS:
	void samplingFreqAvailable(int samplingFreq);
	void onAvailableInfo(const OutputInfo &outInfo, QList<QIQPlotInfo> plotInfoList);
	void onAnalysisConfig(const QString &type, const QVariantMap &config, const OutputInfo &outInfo);
	void onDataIsProcessed(int samplesOffset, int samplesCount);
	void updatePlots();

private:
	void updateAxis(int samples);
	PlotWidget *createPlotWidget(QIQPlotInfo plotInfo);
	void setupPlots(QList<QIQPlotInfo> plotInfoList);
	void setupDataManager(const OutputInfo &outInfo);

	int m_samplingFreq = 512;
	DataManager *m_dataManager;
	QVector<QIQPlotInfo> m_plotsInfo;
	QVector<PlotWidget *> m_plots;
};

} // namespace scopy::qiqplugin

#endif // PLOTMANAGER_H
