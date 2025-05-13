/*
 * Copyright (c) 2025 Analog Devices Inc.
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
#include "animatedrefreshbtn.h"

#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_GUI_ANIMATION_REFRESH_BTN, "AnimatedRefreshButton")

using namespace scopy;

AnimatedRefreshBtn::AnimatedRefreshBtn(bool small, QWidget *parent)
	: AnimationPushButton{parent}
{
	setIcon(Style::getPixmap(":/gui/icons/refresh.svg", Style::getColor(json::theme::content_inverse)));
	setText("Refresh");

	QMovie *movie = new QMovie(":/gui/loading.gif");
	setAnimation(movie, 20000);
	setAutoDefault(true);

	setIsSmallBtn(small);
}

void AnimatedRefreshBtn::setIsSmallBtn(bool isSmall)
{
	if(isSmall) {
		setIconSize(QSize(25, 25));
		Style::setStyle(this, style::properties::button::basicButton);
	} else {
		setIconSize(QSize(30, 30));
		Style::setStyle(this, style::properties::button::basicButtonBig);
	}
}

#include "moc_animatedrefreshbtn.cpp"
