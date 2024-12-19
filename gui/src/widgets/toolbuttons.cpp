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

InfoBtn::InfoBtn(QWidget *parent, bool hasTutorial)
	: QPushButton(parent)
	, m_hasTutorial(hasTutorial)
{

	m_iconPath = ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/info.svg";
	setIcon(Style::getPixmap(m_iconPath, Style::getColor(json::theme::content_default)));

	setCheckable(false);
	Style::setStyle(this, style::properties::button::squareIconButton);
	installEventFilter(this);
}

bool InfoBtn::eventFilter(QObject *watched, QEvent *event)
{
	QPushButton *button = qobject_cast<QPushButton *>(watched);
	if(!button) {
		return false;
	}

	if(event->type() == QEvent::Enter) {
		setIcon(Style::getPixmap(m_iconPath, Style::getColor(json::theme::content_inverse)));
		return true;
	}

	if(event->type() == QEvent::Leave) {
		setIcon(Style::getPixmap(m_iconPath, Style::getColor(json::theme::content_default)));
		return true;
	}

	return false;
}

bool InfoBtn::hasTutorial() { return m_hasTutorial; }

void InfoBtn::generateInfoPopup(QWidget *parent)
{
	m_popupWidget = new PopupWidget(parent);
	m_popupWidget->enableCenterOnParent(true);
	m_popupWidget->setTitle("Plugin Information");
	m_popupWidget->setDescription(
		"To learn more about this plugin, check out the tutorial or read the online documentation.");
	m_popupWidget->getExitBtn()->setText("Tutorial");
	m_popupWidget->getContinueBtn()->setText("Documentation");
	m_popupWidget->enableCloseButton(true);

	connect(m_popupWidget->getExitBtn(), &QPushButton::clicked, this, [=]() { m_popupWidget->deleteLater(); });

	connect(m_popupWidget->getContinueBtn(), &QPushButton::clicked, this, [=]() { m_popupWidget->deleteLater(); });

	m_popupWidget->enableTintedOverlay(true);
	m_popupWidget->show();
	m_popupWidget->raise();
}

QPushButton *InfoBtn::getTutorialButton()
{
	if(m_hasTutorial) {
		return m_popupWidget->getExitBtn();
	}

	return nullptr;
}

QPushButton *InfoBtn::getDocumentationButton()
{
	if(m_hasTutorial) {
		return m_popupWidget->getContinueBtn();
	}
	return nullptr;
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
