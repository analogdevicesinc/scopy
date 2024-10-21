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

#ifndef MENUCOMBO_H
#define MENUCOMBO_H

#include <QComboBox>
#include <QLabel>
#include <QPen>
#include <QWidget>

#include <scopy-gui_export.h>
#include <utils.h>
#include <mousewheelwidgetguard.h>

namespace scopy {

class SCOPY_GUI_EXPORT MenuCombo : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuCombo(QString title, QWidget *parent = nullptr);
	virtual ~MenuCombo();

	QComboBox *combo();

private:
	QLabel *m_label;
	QComboBox *m_combo;
	MouseWheelWidgetGuard *m_mouseWheelGuard;
};

class SCOPY_GUI_EXPORT MenuComboWidget : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuComboWidget(QString title, QWidget *parent = nullptr);
	virtual ~MenuComboWidget();

	QComboBox *combo();

private:
	MenuCombo *m_combo;
	QPen m_pen;
};
} // namespace scopy

#endif // MENUCOMBO_H
