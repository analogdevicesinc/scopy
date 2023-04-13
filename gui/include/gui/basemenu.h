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

#ifndef BASEMENU_H
#define BASEMENU_H

#include <QWidget>
#include "scopygui_export.h"
#include "basemenuitem.h"

namespace Ui {
class BaseMenu;
}

namespace scopy {
class SCOPYGUI_EXPORT BaseMenu : public QWidget
{
	Q_OBJECT

public:
	explicit BaseMenu(QWidget *parent = nullptr);
	virtual ~BaseMenu();

	void insertMenuItem(BaseMenuItem *menuItem, int position = -1);
	void insertMenuItem(QVector<BaseMenuItem *> items, QVector<int> positions = QVector<int>());
	void removeMenuItem(BaseMenuItem *menuItem);
	void removeMenuItem(QVector<BaseMenuItem *> items);

	int positionOf(BaseMenuItem* menuItem);

	void setMargins(int left, int top, int right, int bottom);

	int spacing() const;
	void setSpacing(int spacing);

Q_SIGNALS:
	void itemMovedFromTo(short, short);

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dropEvent(QDropEvent *event);

public Q_SLOTS:
	void moveItem(short from, short to);

protected:
	Ui::BaseMenu *d_ui;
	int d_items;
	void _updateItemsPosition();
};
}

#endif // BASEMENU_H
