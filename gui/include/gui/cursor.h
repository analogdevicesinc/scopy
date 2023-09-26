#ifndef CURSOR_H
#define CURSOR_H
#include <QObject>
#include "plotwidget.h"
#include "scopy-gui_export.h"


namespace scopy {

class SCOPY_GUI_EXPORT Cursor : public QObject {
	Q_OBJECT
public:
	Cursor(PlotWidget* p, PlotAxis *ax, QPen pen);
	~Cursor();
	PlotLineHandle *cursorHandle() const;
	bool isVisible();

public Q_SLOTS:
	void setAxis(PlotAxis *ax);
	void setVisible(bool visible);
	virtual void setCanLeavePlot(bool leave);

Q_SIGNALS:
	void positionChanged(double);

protected:
	PlotWidget *m_plotWidget;
	PlotAxis *m_axis;
	QwtPlot *m_plot;
	QPen m_pen;
	SymbolController* m_symbolCtrl;
	PlotLineHandle* m_cursorHandle;
	Symbol* m_cursorBar;
	bool m_canLeavePlot;

};
}

#endif // CURSOR_H
