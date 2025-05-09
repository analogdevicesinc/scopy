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

#ifndef MONITORPLOTCURVE_HPP
#define MONITORPLOTCURVE_HPP

#include <QObject>
#include <plotchannel.h>
#include "datamonitor/datamonitormodel.hpp"
#include "scopy-datalogger_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGER_EXPORT MonitorPlotCurve : public QObject
{
	Q_OBJECT
public:
	explicit MonitorPlotCurve(DataMonitorModel *dataMonitorModel, PlotWidget *plot, QObject *parent = nullptr);
	~MonitorPlotCurve();

	void clearCurveData();
	void refreshCurve();
	void toggleActive(bool toggled);

	PlotChannel *plotch() const;
	double curveMinVal();
	double curveMaxVal();

private:
	PlotWidget *m_plot;
	PlotChannel *m_plotch;
	PlotAxis *chPlotAxis;
	DataMonitorModel *m_dataMonitorModel;
};
} // namespace datamonitor
} // namespace scopy

#endif // MONITORPLOTCURVE_HPP
