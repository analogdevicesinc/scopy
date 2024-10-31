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

#ifndef PLOTTIMEAXISCONTROLLER_HPP
#define PLOTTIMEAXISCONTROLLER_HPP

#include <QDateTimeEdit>
#include <QWidget>
#include <menuonoffswitch.h>
#include <monitorplot.hpp>
#include <spinbox_a.hpp>
#include "scopy-datalogger_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGER_EXPORT PlotTimeAxisController : public QWidget
{
	Q_OBJECT
public:
	explicit PlotTimeAxisController(MonitorPlot *m_plot, QWidget *parent = nullptr);

	void togglePlotNow(bool toggled);
	void updatePlotStartPoint();

signals:

private:
	MonitorPlot *m_plot;
	PositionSpinButton *m_xdelta;
	QDateEdit *dateEdit;
	QTimeEdit *timeEdit;
	MenuOnOffSwitch *realTimeToggle;
	MenuOnOffSwitch *livePlottingToggle;
};
} // namespace datamonitor
} // namespace scopy
#endif // PLOTTIMEAXISCONTROLLER_HPP
