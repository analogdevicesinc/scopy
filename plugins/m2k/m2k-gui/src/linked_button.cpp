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

#include "linked_button.hpp"

#include "dynamicWidget.h"

#include <QDesktopServices>
#include <QFile>
#include <QUrl>
#include <style.h>

using namespace scopy;

LinkedButton::LinkedButton(QWidget *parent)
	: QPushButton(parent)
{
	Style::setStyle(this, style::properties::button::darkGrayButton, true, true);
	this->setIcon(Style::getPixmap(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
				       "/icons/info.svg"));
	this->setIconSize(QSize(40, 40));

	this->setToolTip("See more info");

	connect(this, SIGNAL(clicked()), this, SLOT(openUrl()));
}

void LinkedButton::openUrl() { QDesktopServices::openUrl(QUrl(this->url)); }

void LinkedButton::setUrl(QString url) { this->url = url; }

#include "moc_linked_button.cpp"
