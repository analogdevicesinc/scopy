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

#ifndef MENUBIGSWITCH_H
#define MENUBIGSWITCH_H

#include <QHBoxLayout>
#include <QWidget>

#include <customSwitch.h>
#include <scopy-gui_export.h>
#include <stylehelper.h>
#include <utils.h>

namespace scopy {

class SCOPY_GUI_EXPORT MenuBigSwitch : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuBigSwitch(QString on, QString off, QWidget *parent = nullptr);
	virtual ~MenuBigSwitch();

	CustomSwitch *onOffswitch();
	void applyStylesheet();

private:
	CustomSwitch *m_switch;
};
} // namespace scopy
#endif // MENUBIGSWITCH_H
