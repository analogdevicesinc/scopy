#ifndef CURSORCONTROLLER_H
#define CURSORCONTROLLER_H

#include <QObject>
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

	CursorSettings *getCursorSettings();

public Q_SLOTS:
	void setVisible(bool visible);
	void readoutsSetVisible(bool visible);
	void cursorsSetVisible(bool visible);
	void horizEnToggled(bool toggled);
	void horizLockToggled(bool toggled);
	void horizTrackToggled(bool toggled);
	void vertEnToggled(bool toggled);
	void vertLockToggled(bool toggled);
	void readoutsDragToggled(bool toggled);

private:
	PlotWidget *m_plot;
	PlotCursors *plotCursors;
	CursorSettings *cursorSettings;
	PlotCursorReadouts *plotCursorReadouts;
	HoverWidget *hoverReadouts;
	double horizLockGap;
	double vertLockGap;
	VCursor *v1Cursor;
	VCursor *v2Cursor;
	HCursor *h1Cursor;
	HCursor *h2Cursor;
	bool horizEn, horizLock, horizTrack;
	bool vertEn, vertLock;
	bool readoutDragsEn;

	void initUI();
	void connectSignals();
	void initSession();
};
} // namespace scopy
#endif // CURSORCONTROLLER_H
