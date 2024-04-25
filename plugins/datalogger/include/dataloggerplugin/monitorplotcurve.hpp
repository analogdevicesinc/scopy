#ifndef MONITORPLOTCURVE_HPP
#define MONITORPLOTCURVE_HPP

#include <QObject>
#include <plotchannel.h>
#include "datamonitor/datamonitormodel.hpp"
#include "scopy-dataloggerplugin_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGERPLUGIN_EXPORT MonitorPlotCurve : public QObject
{
	Q_OBJECT
public:
	explicit MonitorPlotCurve(DataMonitorModel *dataMonitorModel, PlotWidget *plot, QObject *parent = nullptr);
	~MonitorPlotCurve();

	void clearCurveData();
	void refreshCurve();
	void toggleActive(bool toggled);

	PlotChannel *plotch() const;
	double curveMinVal();
	double curveMaxVal();

private:
	PlotWidget *m_plot;
	PlotChannel *m_plotch;
	PlotAxis *chPlotAxis;
	DataMonitorModel *m_dataMonitorModel;
};
} // namespace datamonitor
} // namespace scopy

#endif // MONITORPLOTCURVE_HPP
