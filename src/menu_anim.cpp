/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "menu_anim.hpp"

#include <QSignalTransition>
#include <QSizePolicy>

using namespace adiscope;

MenuAnim::MenuAnim(QWidget *parent) : ColoredQWidget(parent),
	open_anim_max(this, "maximumWidth"),
	open_anim_min(this, "minimumWidth"),
	close_anim_max(this, "maximumWidth"),
	close_anim_min(this, "minimumWidth"),
	min_width(-1),
	animInProg(false)
{
	open_anim_max.setDuration(500);
	open_anim_max.setEasingCurve(QEasingCurve::InOutExpo);

	open_anim_min.setDuration(500);
	open_anim_min.setEasingCurve(QEasingCurve::InOutExpo);

	close_anim_max.setDuration(500);
	close_anim_max.setEasingCurve(QEasingCurve::InOutExpo);

	close_anim_min.setDuration(500);
	close_anim_min.setEasingCurve(QEasingCurve::InOutExpo);

	connect(&open_anim_max, SIGNAL(finished()),
			this, SLOT(openAnimFinished()));
	connect(&close_anim_max, SIGNAL(finished()),
			this, SLOT(closeAnimFinished()));
}

void MenuAnim::toggleMenu(bool open)
{
	int start, stop;

	animInProg = true;

	if (min_width < 0)
		min_width = 0;

	if (open) {
		start = min_width;
		stop = sizeHint().width();

		close_anim_max.stop();
		close_anim_min.stop();

		open_anim_max.setStartValue(start);
		open_anim_max.setEndValue(stop);
		open_anim_max.start();

		open_anim_min.setStartValue(start);
		open_anim_min.setEndValue(stop);
		open_anim_min.start();
	} else {
		start = sizeHint().width();
		stop = min_width;

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

void MenuAnim::setMinimumSize(QSize size)
{
	QWidget::setMinimumSize(size);

	/* Memorize the min width set in the .ui file */
	if (min_width < 0)
		min_width = size.width();
}

bool MenuAnim::animInProgress() const
{
	return animInProg;
}

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
