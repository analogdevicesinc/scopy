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
#include "animatedloadingbutton.h"

#include <QLoggingCategory>
#include <style.h>
#include <QMovie>

Q_LOGGING_CATEGORY(CAT_GUI_ANIMATION_LOADING_BTN, "AnimatedLoadingButton")

using namespace scopy;

AnimatedLoadingButton::AnimatedLoadingButton(const QString &text, QWidget *parent)
	: AnimationPushButton{parent}
{
	setText(text);

	QMovie *movie = new QMovie(":/gui/loading.gif");
	setAnimation(movie, 20000);
	setAutoDefault(true);

	Style::setStyle(this, style::properties::button::basicButton);
}

#include "moc_animatedloadingbutton.cpp"
