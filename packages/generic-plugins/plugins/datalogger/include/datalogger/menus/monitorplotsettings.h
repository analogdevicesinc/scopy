/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef MONITORPLOTSETTINGS_H
#define MONITORPLOTSETTINGS_H

#include <QWidget>
#include <monitorplot.hpp>
#include <spinbox_a.hpp>
#include "scopy-datalogger_export.h"
#include <plotautoscaler.h>

namespace scopy {
namespace datamonitor {
class SCOPY_DATALOGGER_EXPORT MonitorPlotSettings : public QWidget
{
	Q_OBJECT
public:
	explicit MonitorPlotSettings(MonitorPlot *m_plot, QWidget *parent = nullptr);

public Q_SLOTS:
	void plotYAxisMinValueUpdate(double value);
	void plotYAxisMaxValueUpdate(double value);
	void toggleDeleteButtonVisible(bool visible);

Q_SIGNALS:
	void curveStyleIndexChanged(int index);
	void changeCurveThickness(double thickness);

	void plotYAxisAutoscale(bool toggled);
	void plotYAxisMinValueChange(double value);
	void plotYAxisMaxValueChange(double value);
	void plotXAxisMinValueChange(double value);
	void plotXAxisMaxValueChange(double value);
	void requestYMinMaxValues();
	void requestDeletePlot(uint32_t uuid);

private:
	bool m_isDeletable;
	PositionSpinButton *m_ymin;
	PositionSpinButton *m_ymax;
	QWidget *generateEditTitleSettings(QWidget *parent);
	QWidget *generateYAxisSettings(QWidget *parent);
	QWidget *generateCurveStyleSettings(QWidget *parent);
	QWidget *generatePlotUiSettings(QWidget *parent);

	QVBoxLayout *mainLayout;

	MonitorPlot *m_plot;
	QPushButton *m_deletePlot;
};
} // namespace datamonitor
} // namespace scopy

#endif // MONITORPLOTSETTINGS_H
