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

#ifndef GENERICMENU_HPP
#define GENERICMENU_HPP

#include "scopy-m2k-gui_export.h"
#include "basemenu.h"
#include "menu_header.hpp"

#include <subsection_separator.hpp>

#include <QWidget>

namespace scopy {
namespace m2kgui {

class SCOPY_M2K_GUI_EXPORT GenericMenu : public QWidget
{
	Q_OBJECT

public:
	explicit GenericMenu(QWidget *parent = nullptr);
	~GenericMenu();

	virtual void setMenuButton(bool toggled){};

	void initInteractiveMenu();
	void setMenuHeader(const QString &title, const QColor *lineColor, bool hasEnableBtn);
	void insertSection(scopy::m2kgui::SubsectionSeparator *section);
	void addNewHeaderWidget(QWidget *widget);

	void setMenuWidget(QWidget *widget);
	void hideEvent(QHideEvent *) override;
	void showEvent(QShowEvent *) override;

Q_SIGNALS:
	void enableBtnToggled(bool toggled);
	void menuVisibilityChanged(bool toggled);

private:
	MenuHeader *m_menuHeader;
	BaseMenu *m_menu;

	int m_lastOpenPosition;
};
} // namespace m2kgui
} // namespace scopy

#endif // GENERICMENU_HPP
