#ifndef MONITORPLOT_H
#define MONITORPLOT_H

#include <QWidget>
#include <QMap>
#include <plotwidget.h>
#include <plotaxis.h>
#include <datamonitormodel.hpp>

namespace scopy {

class TimePlotInfo;
namespace datamonitor {

struct PlotData
{
	PlotChannel *plotch;
	QVector<double> *ydata;
	QVector<double> *xdata;
};

class MonitorPlot : public QWidget
{
	Q_OBJECT
public:
	explicit MonitorPlot(QWidget *parent = nullptr);

	PlotWidget *plot() const;
	void addMonitor(DataMonitorModel *dataMonitorModel);
	void removeMonitor(QString monitorTitle);
	void toggleMonitor(bool toggled, QString monitorTitle);
	bool hasMonitor(QString title);

	bool firstMonitor() const;
	void setFirstMonitor(bool newFirstMonitor);

	void changeCurveStyle(QString curve, int style);

	void setMainMonitor(QString newMainMonitor);
	void clearMonitor();

private:
	PlotWidget *m_plot;
	TimePlotInfo *m_plotInfo;
	QMap<QString, PlotData *> *m_monitorCurves;
	bool m_firstMonitor = true;
};
} // namespace datamonitor
} // namespace scopy
#endif // MONITORPLOT_H
