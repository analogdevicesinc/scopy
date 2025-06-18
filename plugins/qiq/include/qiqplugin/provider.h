#ifndef PROVIDER_H
#define PROVIDER_H

#include "common/debugtimer.h"
#include "acqsetup.h"
#include "common/scopyconfig.h"
#include <QWidget>
#include <qtimer.h>
#include <dataprovider.h>
#include <plotwidget.h>
#include <toolbuttons.h>
#include <dataacq.h>

namespace scopy::qiqplugin {
class Provider : public QWidget
{
	Q_OBJECT

public:
	Provider(QString uri, QWidget *parent = nullptr);
	~Provider();

public Q_SLOTS:
	void onDataAcqAvailable(QVector<QVector<double>> data, const int &dataSize, const QString &path);

private:
	QString m_uri;
	AcqSetup *m_acqSetup;
	PlotWidget *m_acqPlot;
	PlotWidget *m_receiverPlot;
	RunBtn *m_runBtn;
	DataProvider *m_dataProvider;
	DataAcq *m_dataAcq;
	QVector<double> m_xValues;
	DebugTimer m_testTimer = DebugTimer(scopy::config::settingsFolderPath() + QDir::separator() + "benchmark.csv");

	void initXAxis(PlotWidget *plot, int samples);
	void setupPlotWidget();
	void setupToolTemplate();
	void setupConnections();
	void removePlotChannels(PlotWidget *plot);
	void initChannels(PlotWidget *plot, QStringList chnls);
	void configPressed(AcqSetup::AcqConfig config);
	void configurePlot(PlotWidget *plot, int yMin, int yMax);
	void addPlotChannel(PlotWidget *plot, const QString &label, const QColor &color);
};
} // namespace scopy::qiqplugin

#endif // PROVIDER_H
