#ifndef PLOTCURSORS_H
#define PLOTCURSORS_H
#include "plotwidget.h"
#include "scopy-gui_export.h"
#include <plotaxishandle.h>
#include <QObject>
#include <QPair>

namespace scopy {

class SCOPY_GUI_EXPORT PlotCursors : public QObject
{
	Q_OBJECT
public:
	PlotCursors(PlotWidget *plot);
	~PlotCursors();

	void displayIntersection();
	void setYHandlePos(HandlePos pos);
	void setXHandlePos(HandlePos pos);

	bool tracking() const;

public Q_SLOTS:
	void setVisible(bool visible);
	void setXVisible(bool visible);
	void setYVisible(bool visible);
	PlotAxisHandle *getY1Cursor();
	PlotAxisHandle *getY2Cursor();
	PlotAxisHandle *getX1Cursor();
	PlotAxisHandle *getX2Cursor();
	void setBounded(bool leave);
	void enableTracking(bool tracking);

Q_SIGNALS:
	void update();

private:
	PlotWidget *m_plot;
	QPair<PlotAxisHandle *, PlotAxisHandle *> m_yCursors;
	QPair<PlotAxisHandle *, PlotAxisHandle *> m_xCursors;
	QwtPlotMarker *plotMarker1;
	QwtPlotMarker *plotMarker2;
	bool m_tracking;

	void initUI();
	void connectSignals();
	void updateTracking();
};
} // namespace scopy

#endif // PLOTCURSORS_H
