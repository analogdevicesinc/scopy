#ifndef PLOTAXISHANDLE_H
#define PLOTAXISHANDLE_H

#include "plotwidget.h"
#include <QWidget>
#include "plot_line_handle.h"
#include "scopy-gui_export.h"

namespace scopy {
class SCOPY_GUI_EXPORT PlotAxisHandle : public QObject
{
	Q_OBJECT
public:
	PlotAxisHandle(QPen pen, PlotAxis* ax, PlotWidget *p, QObject *parent = nullptr);
	~PlotAxisHandle();
	RoundedHandleV *handle() const;
	PlotAxis *axis() const;

private:
	PlotWidget *m_plotWidget;
	PlotAxis *m_axis;
	QPen m_pen;
	HorizBar *m_chOffsetBar;
	RoundedHandleV *m_handle;
	SymbolController* m_symbolCtrl;

};
}
#endif // PLOTAXISHANDLE_H
