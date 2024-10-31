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

#ifndef MENULINEEDIT_H
#define MENULINEEDIT_H
#include <QHBoxLayout>
#include <QLineEdit>
#include <QWidget>

#include <scopy-gui_export.h>
#include <stylehelper.h>
#include <utils.h>

namespace scopy {

class SCOPY_GUI_EXPORT MenuLineEdit : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuLineEdit(QWidget *parent = nullptr);
	virtual ~MenuLineEdit();

	QLineEdit *edit();
	void applyStylesheet();

private:
	QLineEdit *m_edit;
};
} // namespace scopy

#endif // MENULINEEDIT_H
