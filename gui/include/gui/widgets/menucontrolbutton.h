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

#ifndef MENUCONTROLBUTTON_H
#define MENUCONTROLBUTTON_H

#include "scopy-gui_export.h"

#include <QAbstractButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include <utils.h>
#include <compositewidget.h>

namespace scopy {

class MenuControlButton;
class SCOPY_GUI_EXPORT MenuControlWidget
{

public:
	virtual MenuControlButton *header() = 0;
};

class SCOPY_GUI_EXPORT MenuControlButton : public QAbstractButton, public MenuControlWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	typedef enum
	{
		CS_CIRCLE,
		CS_SQUARE,
		CS_BLUESQUARE,
		CS_COLLAPSE,
	} CheckboxStyle;

	MenuControlButton(QWidget *parent = nullptr);
	~MenuControlButton();

	void setColor(QColor c);
	void setCheckBoxStyle(MenuControlButton::CheckboxStyle cs);

	void setDoubleClickToOpenMenu(bool b);
	void setOpenMenuChecksThis(bool b);

	QCheckBox *checkBox();
	QPushButton *button();

	MenuControlButton *header() override { return this; }

public Q_SLOTS:
	void setName(QString s);

Q_SIGNALS:
	void doubleClicked();

private:
	void applyStylesheet();

	int width;
	QColor m_color;
	QHBoxLayout *lay;
	QCheckBox *m_chk;
	QLabel *m_label;
	QPushButton *m_btn;
	CheckboxStyle m_cs;
	QMetaObject::Connection dblClickToOpenMenu;
	QMetaObject::Connection openMenuChecksThis;

protected:
	void mouseDoubleClickEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
};

class SCOPY_GUI_EXPORT CollapsableMenuControlButton : public QWidget, public CompositeWidget, public MenuControlWidget
{
	Q_OBJECT
public:
	CollapsableMenuControlButton(QWidget *parent = nullptr);
	~CollapsableMenuControlButton();

	void add(QWidget *ch) override;
	void remove(QWidget *ch) override;
	MenuControlButton *getControlBtn();
	MenuControlButton *header() override { return m_ctrl; }

private:
	MenuControlButton *m_ctrl;
	QWidget *m_container;
	QVBoxLayout *m_contLayout;
	QVBoxLayout *m_lay;
};

} // namespace scopy
#endif // MENUCONTROLBUTTON_H
