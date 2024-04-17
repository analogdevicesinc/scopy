#ifndef PLOTTIMEAXISCONTROLLER_HPP
#define PLOTTIMEAXISCONTROLLER_HPP

#include <QDateTimeEdit>
#include <QWidget>
#include <menuonoffswitch.h>
#include <monitorplot.hpp>
#include <spinbox_a.hpp>
#include "scopy-datamonitorplugin_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATAMONITORPLUGIN_EXPORT PlotTimeAxisController : public QWidget
{
	Q_OBJECT
public:
	explicit PlotTimeAxisController(MonitorPlot *m_plot, QWidget *parent = nullptr);

	void togglePlotNow(bool toggled);
	void updatePlotStartPoint();

signals:

private:
	MonitorPlot *m_plot;
	PositionSpinButton *m_xdelta;
	QDateEdit *dateEdit;
	QTimeEdit *timeEdit;
	MenuOnOffSwitch *realTimeToggle;
	MenuOnOffSwitch *livePlottingToggle;
};
} // namespace datamonitor
} // namespace scopy
#endif // PLOTTIMEAXISCONTROLLER_HPP
