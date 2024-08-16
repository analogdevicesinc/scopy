#ifndef CURSORCONTROLLER_H
#define CURSORCONTROLLER_H

#include "scopy-gui_export.h"
#include <plotcursors.h>
#include <widgets/cursorsettings.h>
#include <widgets/plotcursorreadouts.h>

namespace scopy {
class SCOPY_GUI_EXPORT CursorController : public QObject
{
	Q_OBJECT

public:
	CursorController(PlotWidget *plot, QObject *parent = nullptr);
	~CursorController();

	PlotCursors *getPlotCursors();
	void connectSignals(CursorSettings *cursorSettings);

public Q_SLOTS:
	void setVisible(bool visible);
	void readoutsSetVisible(bool visible);
	void cursorsSetVisible(bool visible);
	void xEnToggled(bool toggled);
	void xLockToggled(bool toggled);
	void xTrackToggled(bool toggled);
	void yEnToggled(bool toggled);
	void yLockToggled(bool toggled);
	void readoutsDragToggled(bool toggled);
	void onAddedChannel(PlotChannel *ch);
	void onRemovedChannel(PlotChannel *ch);
	void updateTracking();

private:
	PlotWidget *m_plot;
	PlotCursors *plotCursors;
	CursorSettings *cursorSettings;
	PlotCursorReadouts *plotCursorReadouts;
	HoverWidget *hoverReadouts;
	double LockGap;
	double yLockGap;
	PlotAxisHandle *y1Cursor;
	PlotAxisHandle *y2Cursor;
	PlotAxisHandle *x1Cursor;
	PlotAxisHandle *x2Cursor;
	bool xEn, xLock, xTrack;
	bool yEn, yLock;
	bool readoutDragsEn;

	void initUI();

	// void initSession();
};
} // namespace scopy
#endif // CURSORCONTROLLER_H
