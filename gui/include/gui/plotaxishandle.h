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

#ifndef PLOTAXISHANDLE_H
#define PLOTAXISHANDLE_H
#include "plotwidget.h"
#include "scopy-gui_export.h"
#include <axishandle.h>
#include <QObject>

namespace scopy {
class AxisHandle;

class SCOPY_GUI_EXPORT PlotAxisHandle : public QWidget
{
	Q_OBJECT
public:
	PlotAxisHandle(PlotWidget *plot, PlotAxis *ax);
	~PlotAxisHandle();

	void setAxis(PlotAxis *axis);
	PlotAxis *axis() const;
	AxisHandle *handle() const;

	double getPosition() const;
	void setPosition(double pos);
	void setPositionSilent(double pos);

	double pixelToScale(int pos);
	int scaleToPixel(double pos);

	void init();

Q_SIGNALS:
	void scalePosChanged(double);
	void updatePos();

private:
	double m_pos;
	PlotWidget *m_plotWidget;
	PlotAxis *m_axis;
	QwtPlot *m_plot;
	AxisHandle *m_handle;
};
} // namespace scopy

#endif // PLOTAXISHANDLE_H
