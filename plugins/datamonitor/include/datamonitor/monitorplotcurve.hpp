#ifndef MONITORPLOTCURVE_HPP
#define MONITORPLOTCURVE_HPP

#include <QObject>
#include <plotchannel.h>
#include "datamonitormodel.hpp"
#include "scopy-datamonitor_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATAMONITOR_EXPORT MonitorPlotCurve : public QObject
{
	Q_OBJECT
public:
	explicit MonitorPlotCurve(DataMonitorModel *dataMonitorModel, PlotWidget *plot, QObject *parent = nullptr);

	void clearCurveData();
	void refreshCurve();
	void toggleActive(bool toggled);
	void setCurveAxisVisible(bool visible);
	void updateAxisIntervalMin(double min);
	void updateAxisIntervalMax(double max);
	void togglePlotAxisVisible(bool toggle);
	void changeCurveStyle(int style);
	const QwtAxisId getAxisId();

	PlotChannel *plotch() const;

signals:

private:
	PlotChannel *m_plotch;
	PlotAxis *chPlotAxis;
	QVector<double> *ydata;
	QVector<double> *xdata;
	double yAxisMin;
	double yAxisMax;
};
} // namespace datamonitor
} // namespace scopy

#endif // MONITORPLOTCURVE_HPP
