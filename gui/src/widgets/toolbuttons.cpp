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

#include <iostream>
#include <style.h>
#include <stylehelper.h>
#include <widgets/toolbuttons.h>

using namespace scopy;

PrintBtn::PrintBtn(QWidget *parent)
	: QPushButton(parent)
{
	Style::setStyle(this, style::properties::button::squareIconButton);
	setFixedWidth(145);
	setCheckable(false);
	setText("Print");
}

OpenLastMenuBtn::OpenLastMenuBtn(MenuHAnim *menu, bool opened, QWidget *parent)
	: QPushButton(parent)
	, m_menu(menu)
{
	QString iconPath = ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
		"/icons/setup3_unchecked_hover.svg";
	setIcon(Style::getPixmap(iconPath, Style::getColor(json::theme::content_default)));

	connect(this, &QPushButton::toggled, this, [=](bool toggle) {
		const char *color = toggle ? json::theme::content_inverse : json::theme::content_default;
		setIcon(Style::getPixmap(iconPath, Style::getColor(color)));
	});

	setCheckable(true);
	Style::setStyle(this, style::properties::button::squareIconButton);
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
	QString iconPath =
		":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/gear_wheel.svg";
	setIcon(Style::getPixmap(iconPath, Style::getColor(json::theme::content_default)));

	connect(this, &QPushButton::toggled, this, [=](bool toggle) {
		const char *color = toggle ? json::theme::content_inverse : json::theme::content_default;
		setIcon(Style::getPixmap(iconPath, Style::getColor(color)));
	});

	setCheckable(true);
	Style::setStyle(this, style::properties::button::squareIconButton);
}

InfoBtn::InfoBtn(QWidget *parent)
	: QPushButton(parent)
{
	QString iconPath = ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/info.svg";
	setIcon(Style::getPixmap(iconPath, Style::getColor(json::theme::content_default)));

	connect(this, &QPushButton::toggled, this, [=](bool toggle) {
		const char *color = toggle ? json::theme::content_inverse : json::theme::content_default;
		setIcon(Style::getPixmap(iconPath, Style::getColor(color)));
	});

	setCheckable(false);
	Style::setStyle(this, style::properties::button::squareIconButton);
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
	Style::setStyle(this, style::properties::button::runButton);

	QIcon icon1;
	icon1.addPixmap(Style::getPixmap(":/gui/icons/play.svg", Style::getColor(json::theme::content_inverse)),
			QIcon::Normal, QIcon::Off);
	icon1.addPixmap(Style::getPixmap(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
						 "/icons/play_stop.svg",
					 Style::getColor(json::theme::content_inverse)),
			QIcon::Normal, QIcon::On);
	setIcon(icon1);
}

SingleShotBtn::SingleShotBtn(QWidget *parent)
	: QPushButton(parent)
{
	setObjectName("single_btn");
	setCheckable(true);
	setChecked(false);
	setText("Single");
	connect(this, &QPushButton::toggled, this, [=](bool b) { setText(b ? "Stop" : "Single"); });
	Style::setStyle(this, style::properties::button::singleButton);
	setStyleSheet("background-color: #ff7200;");

	QIcon icon1;
	icon1.addPixmap(Style::getPixmap(":/gui/icons/play_oneshot.svg", Style::getColor(json::theme::content_inverse)),
			QIcon::Normal, QIcon::Off);
	icon1.addPixmap(Style::getPixmap(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
						 "/icons/play_stop.svg",
					 Style::getColor(json::theme::content_inverse)),
			QIcon::Normal, QIcon::On);
	setIcon(icon1);
}

#include "moc_toolbuttons.cpp"

AddBtn::AddBtn(QWidget *parent)
{
	QString iconPath = ":/gui/icons/add.svg";
	setIcon(Style::getPixmap(iconPath, Style::getColor(json::theme::content_default)));

	connect(this, &QPushButton::toggled, this, [=](bool toggle) {
		const char *color = toggle ? json::theme::content_inverse : json::theme::content_default;
		setIcon(Style::getPixmap(iconPath, Style::getColor(color)));
	});

	setCheckable(false);
	Style::setStyle(this, style::properties::button::squareIconButton);
}

RemoveBtn::RemoveBtn(QWidget *parent)
{
	QString iconPath = ":/gui/icons/red_x.svg";
	setIcon(Style::getPixmap(iconPath, Style::getColor(json::theme::content_default)));

	connect(this, &QPushButton::toggled, this, [=](bool toggle) {
		const char *color = toggle ? json::theme::content_inverse : json::theme::content_default;
		setIcon(Style::getPixmap(iconPath, Style::getColor(color)));
	});

	setCheckable(false);
	Style::setStyle(this, style::properties::button::squareIconButton);
}

SyncBtn::SyncBtn(QWidget *parent)
{
	QIcon icon1;
	icon1.addPixmap(Style::getPixmap(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
					 "/icons/gear_wheel.svg"));
	Style::setStyle(this, style::properties::button::squareIconButton);
	setText("Sync");
	setCheckable(true);
	setIcon(icon1);
}
