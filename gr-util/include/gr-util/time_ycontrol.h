#ifndef TIMEYCONTROL_H
#define TIMEYCONTROL_H

#include "plotaxis.h"
#include "scopy-gr-util_export.h"
#include "spinbox_a.hpp"

#include <QWidget>

namespace scopy::grutil {

class SCOPY_GR_UTIL_EXPORT TimeYControl : public QWidget
{
	Q_OBJECT
public:
	TimeYControl(PlotAxis *, QWidget *parent = nullptr);
	~TimeYControl();
	void setMin(double);
	void setMax(double);

private:
	PositionSpinButton *m_ymin;
	PositionSpinButton *m_ymax;
};
} // namespace scopy::grutil

#endif // TIMEYCONTROL_H
