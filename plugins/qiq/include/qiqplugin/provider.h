#ifndef PROVIDER_H
#define PROVIDER_H

#include "common/debugtimer.h"
#include "dataacquisition.h"
#include <QWidget>
#include <qtimer.h>
#include <dataprovider.h>
#include <plotwidget.h>
#include <toolbuttons.h>

namespace scopy::qiqplugin {
class Provider : public QWidget
{
	Q_OBJECT
public:
	Provider(QWidget *parent = nullptr);
	~Provider();

public Q_SLOTS:
	void onDataAcqAvailable(const double *data, const int &dataSize, const QString &path);

private:
	PlotWidget *m_acqPlot;
	PlotWidget *m_receiverPlot;
	RunBtn *m_runBtn;
	DataProvider *m_dataProvider;
	DataAcquisition *m_dataAcq;
	QVector<double> m_xValues;
	DebugTimer m_testTimer;

	void setupPlotWidget();
	void setupToolTemplate();
	void setupConnections();
	void configurePlot(PlotWidget *plot, int yMin, int yMax);
	void addPlotChannel(PlotWidget *plot, const QString &label, const QColor &color);
};
} // namespace scopy::qiqplugin

#endif // PROVIDER_H
