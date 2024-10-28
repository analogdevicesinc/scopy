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
	PlotCursors(PlotWidget *plot, QObject *parent = nullptr);
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
