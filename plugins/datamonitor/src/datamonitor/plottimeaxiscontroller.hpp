#ifndef PLOTTIMEAXISCONTROLLER_HPP
#define PLOTTIMEAXISCONTROLLER_HPP

#include <QDateTimeEdit>
#include <QWidget>
#include <menuonoffswitch.h>
#include <monitorplot.hpp>
#include <spinbox_a.hpp>

namespace scopy {
namespace datamonitor {

class PlotTimeAxisController : public QWidget
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
	MenuOnOffSwitch *plotNowToggle;
};
} // namespace datamonitor
} // namespace scopy
#endif // PLOTTIMEAXISCONTROLLER_HPP
