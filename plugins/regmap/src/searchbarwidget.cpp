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

#include "searchbarwidget.hpp"
#include "utils.hpp"
#include "utils.h"

#include "regmapstylehelper.hpp"

#include <qboxlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <style.h>
#include <stylehelper.h>
#include "regmapstylehelper.hpp"

using namespace scopy;
using namespace regmap;

SearchBarWidget::SearchBarWidget(QWidget *parent)
	: QWidget{parent}
{
	layout = new QHBoxLayout();
	Utils::removeLayoutMargins(layout);
	setLayout(layout);

	searchBar = new QLineEdit();
	searchBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	searchBar->setPlaceholderText("Search for register ");
	searchButton = new QPushButton(this);

	QString iconPath = ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/search.svg";
	searchButton->setIcon(Style::getPixmap(iconPath, Style::getColor(json::theme::content_default)));

	connect(searchButton, &QPushButton::toggled, this, [=](bool toggle) {
		const char *color = toggle ? json::theme::content_inverse : json::theme::content_default;
		searchButton->setIcon(Style::getPixmap(iconPath, Style::getColor(color)));
	});
	StyleHelper::SquareToggleButtonWithIcon(searchButton, "search_btn", false);

	QObject::connect(searchBar, &QLineEdit::returnPressed, searchButton, &QPushButton::pressed);

	QObject::connect(searchButton, &QPushButton::pressed, this,
			 [=]() { Q_EMIT requestSearch(searchBar->text().toLower()); });
	layout->addWidget(searchBar);
	layout->addWidget(searchButton);

	applyStyle();
}

SearchBarWidget::~SearchBarWidget()
{
	delete layout;
	delete searchBar;
	delete searchButton;
}

void SearchBarWidget::setEnabled(bool enabled)
{
	this->searchBar->setEnabled(enabled);
	this->searchButton->setVisible(enabled);
}

void SearchBarWidget::applyStyle() { RegmapStyleHelper::searchBarStyle(this); }
