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

#ifndef QWT_UTILS_H
#define QWT_UTILS_H

#include "scopy-m2k-gui_export.h"
#include <QDockWidget>
#include <QMainWindow>
#include <QWidget>
#include <qevent.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_picker.h>

class SCOPY_M2K_GUI_EXPORT QwtDblClickPlotPicker : public QwtPlotPicker
{
public:
#if QWT_VERSION < 0x060100
	QwtDblClickPlotPicker(QwtPlotCanvas *);
#else  /* QWT_VERSION < 0x060100 */
	QwtDblClickPlotPicker(QWidget *);
#endif /* QWT_VERSION < 0x060100 */

	~QwtDblClickPlotPicker();

	virtual QwtPickerMachine *stateMachine(int) const;
};

class SCOPY_M2K_GUI_EXPORT QwtPickerDblClickPointMachine : public QwtPickerMachine
{
public:
	QwtPickerDblClickPointMachine();
	~QwtPickerDblClickPointMachine();

#if QWT_VERSION < 0x060000
	virtual CommandList
#else
	virtual QList<QwtPickerMachine::Command>
#endif
	transition(const QwtEventPattern &eventPattern, const QEvent *e);
};

#endif // QWT_UTILS_H
