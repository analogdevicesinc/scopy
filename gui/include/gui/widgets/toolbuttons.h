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

#ifndef TOOLBUTTONS_H
#define TOOLBUTTONS_H

#include "semiexclusivebuttongroup.h"

#include <QPushButton>

#include <menu_anim.hpp>
#include <scopy-gui_export.h>
#include <utils.h>

namespace scopy {

class SCOPY_GUI_EXPORT PrintBtn : public QPushButton
{
	Q_OBJECT
public:
	PrintBtn(QWidget *parent = nullptr);
};

class SCOPY_GUI_EXPORT OpenLastMenuBtn : public QPushButton
{
	Q_OBJECT
public:
	OpenLastMenuBtn(MenuHAnim *menu, bool opened, QWidget *parent = nullptr);
	QButtonGroup *getButtonGroup();

private:
	MenuHAnim *m_menu;
	SemiExclusiveButtonGroup *grp;
};

/* Refactor these as Stylehelper ? */
class SCOPY_GUI_EXPORT GearBtn : public QPushButton
{
	Q_OBJECT
public:
	GearBtn(QWidget *parent = nullptr);
};

class SCOPY_GUI_EXPORT InfoBtn : public QPushButton
{
	Q_OBJECT
public:
	InfoBtn(QWidget *parent = nullptr);
};

class SCOPY_GUI_EXPORT RunBtn : public QPushButton
{
	Q_OBJECT
public:
	RunBtn(QWidget *parent = nullptr);
};

class SCOPY_GUI_EXPORT SingleShotBtn : public QPushButton
{
	Q_OBJECT
public:
	SingleShotBtn(QWidget *parent = nullptr);
};

class SCOPY_GUI_EXPORT AddBtn : public QPushButton
{
	Q_OBJECT
public:
	AddBtn(QWidget *parent = nullptr);
};

class SCOPY_GUI_EXPORT RemoveBtn : public QPushButton
{
	Q_OBJECT
public:
	RemoveBtn(QWidget *parent = nullptr);
};

class SCOPY_GUI_EXPORT SyncBtn : public QPushButton
{
	Q_OBJECT
public:
	SyncBtn(QWidget *parent = nullptr);
};

} // namespace scopy

#endif
