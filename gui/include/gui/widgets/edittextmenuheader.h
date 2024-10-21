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

#ifndef EDITTEXTMENUHEADER_H
#define EDITTEXTMENUHEADER_H

#include <QWidget>
#include <QFrame>
#include <QLineEdit>

#include <utils.h>
#include <scopy-gui_export.h>

namespace scopy {
class SCOPY_GUI_EXPORT EditTextMenuHeader : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	explicit EditTextMenuHeader(QString title, QPen pen, QWidget *parent = nullptr);
	~EditTextMenuHeader();

	QLineEdit *lineEdit() const;
	void applyStylesheet();

private:
	QFrame *m_line;
	QPen m_pen;
	QLineEdit *m_lineEdit;
};
} // namespace scopy
#endif // EDITTEXTMENUHEADER_H
