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

#ifndef MENU_HEADER_HPP
#define MENU_HEADER_HPP

#include "scopy-m2k-gui_export.h"
#include <QPushButton>
#include <QWidget>

namespace Ui {
class MenuHeader;
}

namespace scopy {
namespace m2kgui {

class SCOPY_M2K_GUI_EXPORT MenuHeader : public QWidget
{
	Q_OBJECT

public:
	explicit MenuHeader(QWidget *parent = nullptr);
	explicit MenuHeader(const QString &label = nullptr, const QColor *color = new QColor("#4A64FF"),
			    bool enableBtnVisible = false, QWidget *parent = nullptr);
	~MenuHeader();

private:
	Ui::MenuHeader *m_ui;

public Q_SLOTS:
	void setEnabledBtnState(bool state);

Q_SIGNALS:
	void enableBtnToggled(bool state);

public:
	void setLabel(const QString &text);
	void setLineColor(const QColor *color);
	void setEnableBtnVisible(bool visible);

	QPushButton *getEnableBtn();
	void addNewHeaderWidget(QWidget *widget);
};
} // namespace m2kgui
} // namespace scopy

#endif // MENU_HEADER_HPP
