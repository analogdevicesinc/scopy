/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

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

	void static syncXCursorControllers(CursorController *ctrl1, CursorController *ctrl2);
	void static unsyncXCursorControllers(CursorController *ctrl1, CursorController *ctrl2);
	bool isVisible();

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

Q_SIGNALS:
	void visibilityChanged(bool visible);

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
	bool m_visible;

	void initUI();
};
} // namespace scopy
#endif // CURSORCONTROLLER_H
