/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef M2K_UTILS_H
#define M2K_UTILS_H

#include <qevent.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <QWidget>

class QwtDblClickPlotPicker: public QwtPlotPicker
{
public:
#if QWT_VERSION < 0x060100
    QwtDblClickPlotPicker(QwtPlotCanvas *);
#else /* QWT_VERSION < 0x060100 */
    QwtDblClickPlotPicker(QWidget *);
#endif /* QWT_VERSION < 0x060100 */

    ~QwtDblClickPlotPicker();

    virtual QwtPickerMachine * stateMachine(int) const;
};

class QwtPickerDblClickPointMachine: public QwtPickerMachine
{
public:
  QwtPickerDblClickPointMachine();
  ~QwtPickerDblClickPointMachine();

#if QWT_VERSION < 0x060000
  virtual CommandList
#else
  virtual QList<QwtPickerMachine::Command>
#endif
    transition( const QwtEventPattern &eventPattern,
				  const QEvent *e);
};

class Util
{
public:
	static void retainWidgetSizeWhenHidden(QWidget *w, bool retain = true);
	static void setWidgetNrOfChars(QWidget *w, int minNrOfChars, int maxNrOfChars=0);
	static void loadStylesheetFromFile(QString path, QWidget* widget);
};

#endif /* M2K_UTILS_H */
