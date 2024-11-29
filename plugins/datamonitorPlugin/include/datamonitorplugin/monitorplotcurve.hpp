#ifndef MONITORPLOTCURVE_HPP
#define MONITORPLOTCURVE_HPP

#include <QObject>
#include <plotchannel.h>
#include "datamonitormodel.hpp"
#include "scopy-datamonitorplugin_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATAMONITORPLUGIN_EXPORT MonitorPlotCurve : public QObject
{
	Q_OBJECT
public:
	explicit MonitorPlotCurve(DataMonitorModel *dataMonitorModel, PlotWidget *plot, QObject *parent = nullptr);

	void clearCurveData();
	void refreshCurve();
	void toggleActive(bool toggled);
	void changeCurveStyle(int style);
	void changeCurveThickness(double thickness);

	PlotChannel *plotch() const;
	int curveStyleIndex() const;
	double curveMinVal();
	double curveMaxVal();

private:
	PlotChannel *m_plotch;
	PlotAxis *chPlotAxis;
	int m_curveStyleIndex;
	DataMonitorModel *m_dataMonitorModel;
};
} // namespace datamonitor
} // namespace scopy

#endif // MONITORPLOTCURVE_HPP
