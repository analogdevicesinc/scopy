#ifndef MENUPLOTAXISRANGECONTROL_H
#define MENUPLOTAXISRANGECONTROL_H

#include "plotaxis.h"
#include "scopy-gui_export.h"
#include "spinbox_a.hpp"

#include <QWidget>

namespace scopy::gui {

class SCOPY_GUI_EXPORT MenuPlotAxisRangeControl : public QWidget
{
	Q_OBJECT
public:
	MenuPlotAxisRangeControl(PlotAxis *, QWidget *parent = nullptr);
	~MenuPlotAxisRangeControl();
	double min();
	double max();
Q_SIGNALS:
	void intervalChanged(double, double);
public Q_SLOTS:
	void setMin(double);
	void setMax(double);

	void addAxis(PlotAxis *ax);
	void removeAxis(PlotAxis *ax);
private:
	PositionSpinButton *m_min;
	PositionSpinButton *m_max;

	QMap<PlotAxis*, QList<QMetaObject::Connection>> connections;
};
} // namespace scopy::gui

#endif // MENUPLOTAXISRANGECONTROL_H
