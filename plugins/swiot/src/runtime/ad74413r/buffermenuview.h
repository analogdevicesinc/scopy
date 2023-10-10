/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#ifndef SWIOTGENERICMENU_H
#define SWIOTGENERICMENU_H

#include "buffermenu.h"
#include "linked_button.hpp"

#include <QMap>
#include <QObject>
#include <QWidget>

#include <gui/generic_menu.hpp>

namespace scopy {
class CustomSwitch;
class GenericMenu;

namespace swiot {
class BufferMenuView : public gui::GenericMenu
{
	Q_OBJECT
public:
	explicit BufferMenuView(QWidget *parent = nullptr);

	~BufferMenuView();

	void init(QString title, QString function, QColor *color, QString unit, double yMin, double yMax);
	void initAdvMenu(QMap<QString, QMap<QString, QStringList>> values);
	void createHeaderWidget(const QString title);

	BufferMenu *getAdvMenu();

private:
	bool eventFilter(QObject *obj, QEvent *event);

	BufferMenu *m_swiotAdvMenu;
	scopy::gui::SubsectionSeparator *m_advanceSettingsSection;
	scopy::gui::SubsectionSeparator *m_verticalSettingsSection;
	scopy::LinkedButton *m_btnInfoStatus;
};
} // namespace swiot
} // namespace scopy

#endif // SWIOTGENERICMENU_H
