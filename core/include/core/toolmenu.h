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

#ifndef TOOLMENU_H
#define TOOLMENU_H

#include <compositewidget.h>
#include <QMap>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QButtonGroup>

namespace scopy {

class ToolMenu : public QWidget, public CompositeWidget
{
	Q_OBJECT
public:
	ToolMenu(QWidget *parent);
	~ToolMenu();

	void add(QWidget *w) override;
	void add(int index, QString itemId, QWidget *w);
	void remove(QWidget *w) override;
	int indexOf(QWidget *w);
	void colapseAll();

	QButtonGroup *btnGroup() const;

private:
	void add(int index, QWidget *w);
	QString widgetName(QWidget *w);
	void onScrollRangeChanged(int min, int max);

	QMap<QString, QWidget *> m_widgetMap;
	int m_uuid;
	QScrollArea *m_scroll;
	QVBoxLayout *m_layScroll;
	QSpacerItem *m_spacer;
	QButtonGroup *m_btnGroup;
};

} // namespace scopy

#endif // TOOLMENU_H
