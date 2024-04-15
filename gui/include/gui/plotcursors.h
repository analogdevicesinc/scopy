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
	void setVHandlePos(HandlePos pos);
	void setHHandlePos(HandlePos pos);

public Q_SLOTS:
	void setVisible(bool visible);
	void horizSetVisible(bool visible);
	void vertSetVisible(bool visible);
	PlotAxisHandle *getV1Cursor();
	PlotAxisHandle *getV2Cursor();
	PlotAxisHandle *getH1Cursor();
	PlotAxisHandle *getH2Cursor();
	void setBounded(bool leave);
	void enableTracking(bool tracking);

Q_SIGNALS:
	void update();

private:
	PlotWidget *m_plot;
	QPair<PlotAxisHandle *, PlotAxisHandle *> m_vCursors;
	QPair<PlotAxisHandle *, PlotAxisHandle *> m_hCursors;
	QwtPlotMarker *plotMarker1;
	QwtPlotMarker *plotMarker2;
	bool m_tracking;

	void initUI();
	void connectSignals();
	void updateTracking();
	double getHorizIntersectionAt(double pos);
};
} // namespace scopy

#endif // PLOTCURSORS_H
