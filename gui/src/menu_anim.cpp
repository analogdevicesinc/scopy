/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "menu_anim.hpp"

#include <QSignalTransition>
#include <QSizePolicy>

using namespace scopy;

MenuVAnim::MenuVAnim(QWidget *parent)
	: MenuAnim("minimumHeight", "maximumHeight", parent)
{}

int MenuVAnim::getImplicitMin() { return 0; }

int MenuVAnim::getImplicitMax() { return sizeHint().height(); }

MenuHAnim::MenuHAnim(QWidget *parent)
	: MenuAnim("minimumWidth", "maximumWidth", parent)
{}

int MenuHAnim::getImplicitMin() { return 0; }

int MenuHAnim::getImplicitMax() { return sizeHint().width(); }

MenuAnim::MenuAnim(QByteArray minAnimationProperty, QByteArray maxAnimationProperty, QWidget *parent)
	: QWidget(parent)
	, open_anim_max(this, maxAnimationProperty)
	, open_anim_min(this, minAnimationProperty)
	, close_anim_max(this, maxAnimationProperty)
	, close_anim_min(this, minAnimationProperty)
	, min_val(-1)
	, max_val(-1)
	, animInProg(false)
	, animationDuration(200)
{
	setStyleSheet(".QWidget{ background-color: transparent; }");
	setAttribute(Qt::WA_StyledBackground, true);
	open_anim_max.setDuration(animationDuration);
	open_anim_max.setEasingCurve(QEasingCurve::InOutExpo);

	open_anim_min.setDuration(animationDuration);
	open_anim_min.setEasingCurve(QEasingCurve::InOutExpo);

	close_anim_max.setDuration(animationDuration);
	close_anim_max.setEasingCurve(QEasingCurve::InOutExpo);

	close_anim_min.setDuration(animationDuration);
	close_anim_min.setEasingCurve(QEasingCurve::InOutExpo);

	connect(&open_anim_max, SIGNAL(finished()), this, SLOT(openAnimFinished()));
	connect(&close_anim_max, SIGNAL(finished()), this, SLOT(closeAnimFinished()));
}

void MenuAnim::toggleMenu(bool open)
{
	int start, stop;

	animInProg = true;

	int min = min_val < 0 ? getImplicitMin() : min_val;
	int max = max_val < 0 ? getImplicitMax() : max_val;

	if(open) {
		start = min;
		stop = max;

		close_anim_max.stop();
		close_anim_min.stop();

		open_anim_max.setStartValue(start);
		open_anim_max.setEndValue(stop);
		open_anim_max.start();

		open_anim_min.setStartValue(start);
		open_anim_min.setEndValue(stop);
		open_anim_min.start();
	} else {
		start = max;
		stop = min;

		open_anim_max.stop();
		open_anim_min.stop();

		close_anim_max.setStartValue(start);
		close_anim_max.setEndValue(stop);
		close_anim_max.start();

		close_anim_min.setStartValue(start);
		close_anim_min.setEndValue(stop);
		close_anim_min.start();
	}
}

bool MenuAnim::animInProgress() const { return animInProg; }

void MenuAnim::closeAnimFinished()
{
	animInProg = false;
	Q_EMIT finished(false);
}

void MenuAnim::openAnimFinished()
{
	animInProg = false;
	Q_EMIT finished(true);
}

int MenuAnim::getAnimationDuration() const { return animationDuration; }

void MenuAnim::setAnimationDuration(int newAnimationDuration)
{
	animationDuration = newAnimationDuration;
	open_anim_max.setDuration(animationDuration);
	open_anim_min.setDuration(animationDuration);
	close_anim_max.setDuration(animationDuration);
	close_anim_min.setDuration(animationDuration);
}

void MenuAnim::setAnimMin(int min) { min_val = min; }
void MenuAnim::setAnimMax(int max) { max_val = max; }

#include "moc_menu_anim.cpp"
