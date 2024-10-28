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

#ifndef MENUSECTIONWIDGET_H
#define MENUSECTIONWIDGET_H
#include "compositewidget.h"
#include "menucollapsesection.h"
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include <scopy-gui_export.h>
#include <utils.h>

namespace scopy {

class SCOPY_GUI_EXPORT MenuSectionWidget : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuSectionWidget(QWidget *parent = nullptr);
	~MenuSectionWidget();
	QVBoxLayout *contentLayout() const;

private:
	QVBoxLayout *m_layout;
};

class SCOPY_GUI_EXPORT MenuVScrollArea : public QScrollArea
{
	Q_OBJECT
	//	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuVScrollArea(QWidget *parent = nullptr);
	~MenuVScrollArea();
	QVBoxLayout *contentLayout() const;

private:
	QVBoxLayout *m_layout;
};

class SCOPY_GUI_EXPORT MenuSectionCollapseWidget : public QWidget, public Collapsable, public CompositeWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuSectionCollapseWidget(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style,
				  MenuCollapseSection::MenuHeaderWidgetType headerType, QWidget *parent = nullptr);
	~MenuSectionCollapseWidget();
	QVBoxLayout *contentLayout() const;

	void add(QWidget *w) override;
	void remove(QWidget *w) override;

	bool collapsed() override;
	void setCollapsed(bool b) override;
	MenuCollapseSection *collapseSection();

private:
	QVBoxLayout *m_layout;
	MenuSectionWidget *m_section;
	MenuCollapseSection *m_collapse;
};

} // namespace scopy
#endif // MENUSECTIONWIDGET_H
