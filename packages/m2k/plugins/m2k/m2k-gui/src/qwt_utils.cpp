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

#include "qwt_utils.h"

#include <QDebug>
#include <QSizePolicy>

#include <algorithm>
#include <sstream>
#include <string>

QwtPickerDblClickPointMachine::QwtPickerDblClickPointMachine()
#if QWT_VERSION < 0x060000
	: QwtPickerMachine()
#else
	: QwtPickerMachine(PointSelection)
#endif
{}

QwtPickerDblClickPointMachine::~QwtPickerDblClickPointMachine() {}

#if QWT_VERSION < 0x060000
#define CMDLIST_TYPE QwtPickerMachine::CommandList
#else
#define CMDLIST_TYPE QList<QwtPickerMachine::Command>
#endif
CMDLIST_TYPE
QwtPickerDblClickPointMachine::transition(const QwtEventPattern &eventPattern, const QEvent *e)
{
	CMDLIST_TYPE cmdList;
	switch(e->type()) {
	case QEvent::MouseButtonDblClick:
		if(eventPattern.mouseMatch(QwtEventPattern::MouseSelect1, (const QMouseEvent *)e)) {
			cmdList += QwtPickerMachine::Begin;
			cmdList += QwtPickerMachine::Append;
			cmdList += QwtPickerMachine::End;
		}
		break;
	default:
		break;
	}
	return cmdList;
}

#if QWT_VERSION < 0x060100
QwtDblClickPlotPicker::QwtDblClickPlotPicker(QwtPlotCanvas *canvas)
#else  /* QWT_VERSION < 0x060100 */
QwtDblClickPlotPicker::QwtDblClickPlotPicker(QWidget *canvas)
#endif /* QWT_VERSION < 0x060100 */
	: QwtPlotPicker(canvas)
{
#if QWT_VERSION < 0x060000
	setSelectionFlags(QwtPicker::PointSelection);
#endif
}

QwtDblClickPlotPicker::~QwtDblClickPlotPicker() {}

QwtPickerMachine *QwtDblClickPlotPicker::stateMachine(int n) const { return new QwtPickerDblClickPointMachine; }
