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

#include <stylehelper.h>
#include <widgets/toolbuttons.h>

using namespace scopy;

PrintBtn::PrintBtn(QWidget *parent)
	: QPushButton(parent)
{
	StyleHelper::BlueGrayButton(this, "print_btn");
	setFixedWidth(128);
	setCheckable(false);
	setText("Print");
}

OpenLastMenuBtn::OpenLastMenuBtn(MenuHAnim *menu, bool opened, QWidget *parent)
	: QPushButton(parent)
	, m_menu(menu)
{
	QIcon icon1;
	icon1.addPixmap(Util::ChangeSVGColor(":/gui/icons/scopy-default/icons/setup3_unchecked_hover.svg", "white", 1));
	setIcon(icon1);
	StyleHelper::SquareToggleButtonWithIcon(this, "open_last_menu_btn", true);
	setChecked(opened);
	grp = new SemiExclusiveButtonGroup(this);
	connect(this, &QPushButton::toggled, m_menu, &MenuHAnim::toggleMenu);
	connect(grp, &SemiExclusiveButtonGroup::buttonSelected, this, [=](QAbstractButton *btn) {
		if(btn == nullptr) {
			this->setChecked(false);
		} else {
			this->setChecked(true);
		}
	});
	connect(this, &QAbstractButton::toggled, this, [=](bool b) {
		if(b) {
			grp->getLastButton()->setChecked(true);
		} else {
			grp->getLastButton()->setChecked(false);
		}
	});
}

QButtonGroup *OpenLastMenuBtn::getButtonGroup() { return grp; }

GearBtn::GearBtn(QWidget *parent)
	: QPushButton(parent)
{
	QIcon icon1;
	icon1.addPixmap(Util::ChangeSVGColor(":/gui/icons/scopy-default/icons/gear_wheel_hover.svg", "white", 1));
	StyleHelper::SquareToggleButtonWithIcon(this, "gear_btn", true);
	setIcon(icon1);
}

InfoBtn::InfoBtn(QWidget *parent)
	: QPushButton(parent)
{
	QIcon icon1;
	icon1.addPixmap(Util::ChangeSVGColor(":/gui/icons/scopy-default/icons/info.svg", "white", 1));
	StyleHelper::SquareToggleButtonWithIcon(this, "info_btn", false);
	setIcon(icon1);
}

RunBtn::RunBtn(QWidget *parent)
	: QPushButton(parent)
{
	setObjectName("run_btn");
	setCheckable(true);
	setChecked(false);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setText("Run");
	connect(this, &QPushButton::toggled, this, [=](bool b) { setText(b ? "Stop" : "Run"); });
	QString style = QString(R"css(
			QPushButton {
				width: 88px;
				height: 48px;
					border-radius: 2px;
					text-align: left;
					padding-left: 20px;
					padding-right: 20px;
				color: white;
					font-weight: 700;
					font-size: 14px;
			}

			QPushButton:!checked {
				background-color: #27b34f;
			}

			  QPushButton:checked {
				background-color: #F45000;
			}

			QPushButton:disabled {
				background-color: grey;
			})css");

	setStyleSheet(style);
	QIcon icon1;
	icon1.addPixmap(Util::ChangeSVGColor(":/gui/icons/play.svg", "white", 1), QIcon::Normal, QIcon::Off);
	icon1.addPixmap(Util::ChangeSVGColor(":/gui/icons/scopy-default/icons/play_stop.svg", "white", 1),
			QIcon::Normal, QIcon::On);
	setIcon(icon1);
	setLayoutDirection(Qt::RightToLeft);
}

SingleShotBtn::SingleShotBtn(QWidget *parent)
	: QPushButton(parent)
{
	setObjectName("single_btn");
	setCheckable(true);
	setChecked(false);
	setText("Single");
	connect(this, &QPushButton::toggled, this, [=](bool b) { setText(b ? "Stop" : "Single"); });
	QString style = QString(R"css(
			QPushButton {
				width: 88px;
				height: 48px;
					border-radius: 2px;
					text-align: left;
					padding-left: 20px;
					padding-right: 20px;
				color: white;
					font-weight: 700;
					font-size: 14px;
				opacity: 80;
			}

			QPushButton:!checked {
				background-color: #ff7200;
			}

			  QPushButton:checked {
				background-color: #F45000;
			}

			QPushButton:disabled {
				background-color: grey;
			}
			)css");

	setStyleSheet(style);
	QIcon icon1;
	icon1.addPixmap(Util::ChangeSVGColor(":/gui/icons/play_oneshot.svg", "white", 1), QIcon::Normal, QIcon::Off);
	icon1.addPixmap(Util::ChangeSVGColor(":/gui/icons/scopy-default/icons/play_stop.svg", "white", 1),
			QIcon::Normal, QIcon::On);
	setIcon(icon1);
	setLayoutDirection(Qt::RightToLeft);
}

#include "moc_toolbuttons.cpp"

AddBtn::AddBtn(QWidget *parent)
{
	QIcon icon1;
	icon1.addPixmap(Util::ChangeSVGColor(":/gui/icons/launcher_add.svg", "white", 1));
	StyleHelper::SquareToggleButtonWithIcon(this, "add_btn", false);
	setIcon(icon1);
}

RemoveBtn::RemoveBtn(QWidget *parent)
{
	QIcon icon1;
	icon1.addPixmap(Util::ChangeSVGColor(":/gui/icons/red_x.svg", "white", 1));
	StyleHelper::SquareToggleButtonWithIcon(this, "remove_btn", false);
	setIcon(icon1);
}

SyncBtn::SyncBtn(QWidget *parent)
{
	QIcon icon1;
	icon1.addPixmap(Util::ChangeSVGColor(":/gui/icons/scopy-default/icons/gear_wheel_hover.svg", "white", 1));
	StyleHelper::BlueGrayButton(this, "sync_btn");
	setText("Sync");
	setCheckable(true);
	setIcon(icon1);
}
