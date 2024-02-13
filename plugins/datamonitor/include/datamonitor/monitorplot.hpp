#ifndef MONITORPLOT_H
#define MONITORPLOT_H

#include <QWidget>
#include <QMap>
#include <plotwidget.h>
#include <plotaxis.h>
#include <datamonitormodel.hpp>
#include "scopy-datamonitor_export.h"

namespace scopy {

class TimePlotInfo;
namespace datamonitor {

class MonitorPlotCurve;

class SCOPY_DATAMONITOR_EXPORT MonitorPlot : public QWidget
{
	Q_OBJECT
public:
	explicit MonitorPlot(QWidget *parent = nullptr);

	PlotWidget *plot() const;
	void addMonitor(DataMonitorModel *dataMonitorModel);
	void removeMonitor(QString monitorName);
	void toggleMonitor(bool toggled, QString monitorName);
	bool hasMonitor(QString title);

	void changeCurveStyle(QString monitorName, int style);

	void setMainMonitor(QString newMainMonitor);
	void clearMonitor();

	void updateXAxis(int newValue);
	void updateYAxisIntervalMin(QString monitorName, double min);
	void updateYAxisIntervalMax(QString monitorName, double max);

	QString mainMonitor() const;

	void plotYAxisAutoscaleToggled(QString monitorName, bool toggled);

private:
	PlotWidget *m_plot;
	TimePlotInfo *m_plotInfo;
	QMap<QString, MonitorPlotCurve *> *m_monitorCurves;
	bool m_firstMonitor = true;
	QString m_mainMonitor;
};
} // namespace datamonitor
} // namespace scopy
#endif // MONITORPLOT_H
