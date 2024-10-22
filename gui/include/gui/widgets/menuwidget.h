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

#ifndef MENUWIDGET_H
#define MENUWIDGET_H

#include <QWidget>
#include "compositewidget.h"
#include "menuheader.h"
#include "qscrollarea.h"
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QMap>
#include <scopy-gui_export.h>

namespace scopy {
namespace gui {
class SCOPY_GUI_EXPORT MenuWidget : public QWidget, public CompositeWidget
{
	Q_OBJECT
public:
	enum MenuAlignment
	{
		MA_TOPFIRST,
		MA_TOPLAST,
		MA_BOTTOMFIRST,
		MA_BOTTOMLAST
	};

	MenuWidget(QString name, QPen p, QWidget *parent);
	~MenuWidget();

	void add(QWidget *, QString name, MenuAlignment position);
	void add(QWidget *) override;
	void remove(QWidget *) override;
	void add(QWidget *, QString name);
	void remove(QString);
	MenuHeaderWidget *header();

	QWidget *findWidget(QString name);
	QString widgetName(QWidget *);

	void scrollTo(QWidget *);
	void collapseAll();
	void setCollapsed(QString name, bool b);

private:
	QMap<QString, QWidget *> m_widgetMap;
	QSpacerItem *m_spacer;
	QVBoxLayout *m_layScroll;
	MenuHeaderWidget *m_header;
	QScrollArea *scroll;
	int uuid;
};
} // namespace gui

} // namespace scopy

#endif // MENUWIDGET_H
