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

#ifndef MENUPLOTAXISRANGECONTROL_H
#define MENUPLOTAXISRANGECONTROL_H

#include "menuspinbox.h"
#include "plotaxis.h"
#include "scopy-gui_export.h"
#include "spinbox_a.hpp"

#include <QWidget>

namespace scopy::gui {

class SCOPY_GUI_EXPORT MenuPlotAxisRangeControl : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuPlotAxisRangeControl(PlotAxis *, QWidget *parent = nullptr);
	~MenuPlotAxisRangeControl();
	double min();
	double max();
Q_SIGNALS:
	void intervalChanged(double, double);
public Q_SLOTS:
	void setMin(double);
	void setMax(double);

	MenuSpinbox *minSpinbox();
	MenuSpinbox *maxSpinbox();

	void addAxis(PlotAxis *ax);
	void removeAxis(PlotAxis *ax);

private:
	MenuSpinbox *m_min;
	MenuSpinbox *m_max;

	QMap<PlotAxis *, QList<QMetaObject::Connection>> connections;
};
} // namespace scopy::gui

#endif // MENUPLOTAXISRANGECONTROL_H
