#ifndef PLOTCURSORS_H
#define PLOTCURSORS_H
#include <QObject>
#include "plotwidget.h"
#include <QPair>
#include "scopy-gui_export.h"
#include <hcursor.h>
#include <vcursor.h>

namespace scopy {


class SCOPY_GUI_EXPORT PlotCursors : public QObject
{
	Q_OBJECT
public:
	PlotCursors(PlotWidget* plot);
	~PlotCursors();

	void displayIntersection();

public Q_SLOTS:	
	void setVisible(bool visible);
	void horizSetVisible(bool visible);
	void vertSetVisible(bool visible);
	VCursor *getV1Cursor();
	VCursor *getV2Cursor();
	HCursor *getH1Cursor();
	HCursor *getH2Cursor();
	void setCanLeavePlot(bool leave);
	void enableTracking(bool tracking);

Q_SIGNALS:
	void update();

private:
	PlotWidget *m_plot;
	QPair<VCursor*, VCursor*> m_vCursors;
	QPair<HCursor*, HCursor*> m_hCursors;
	QwtPlotMarker *plotMarker1;
	QwtPlotMarker *plotMarker2;
	bool m_tracking;

	void initUI();
	void connectSignals();
	void updateTracking();
	double getHorizIntersectionAt(double pos);
};
}

#endif // PLOTCURSORS_H

