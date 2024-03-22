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
public Q_SLOTS:
	void setMin(double);
	void setMax(double);

private:
	PositionSpinButton *m_min;
	PositionSpinButton *m_max;
};
} // namespace scopy::gui

#endif // MENUPLOTAXISRANGECONTROL_H
