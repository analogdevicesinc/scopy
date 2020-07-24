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

#ifndef BASEMENUITEM_H
#define BASEMENUITEM_H

#include "coloredQWidget.hpp"

#include <QFrame>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

namespace Ui {
class BaseMenuItem;
}

namespace adiscope {
class BaseMenu;
}

namespace adiscope {
class BaseMenuItem : public ColoredQWidget
{
	Q_OBJECT

public:
	explicit BaseMenuItem(QWidget *parent = nullptr);
	virtual ~BaseMenuItem();

	void setWidget(QWidget *widget);

	int position() const;
	void setPosition(int position);

	void setDragWidget(QWidget *widget);

	static const char *menuItemMimeDataType;

	bool eventFilter(QObject *watched, QEvent *event);

	BaseMenu *getOwner() const;
	void setOwner(BaseMenu *menu);

Q_SIGNALS:
	void moveItem(short from, short to);

	/* Emit this signal in derived classes
	 * to know which item gets selected in the
	 * menu
	 */
	void itemSelected();
	void enableInfoWidget(bool);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);

private:

	void _enableBotSeparator(bool enable);
	void _enableTopSeparator(bool enable);

private:
        Ui::BaseMenuItem *d_ui;
	BaseMenu *d_menu;

	int d_position;

	QPoint d_dragStartPosition;
	QRect d_topDragBox;
	QRect d_centerDragBox;
	QRect d_botDragbox;

	QWidget *d_dragWidget;
	bool d_allowDrag;
};
}

#endif // BASEMENUITEM_H
