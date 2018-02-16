/*
 * Copyright 2018 Analog Devices, Inc.
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

#ifndef STACKED_HOMEPAGE_H
#define STACKED_HOMEPAGE_H

#include <QWidget>
#include <QStackedWidget>
#include <QEasingCurve>

#include "homepage_controls.h"

namespace Ui {
class StackedHomepage;
}

namespace adiscope {
class StackedHomepage : public QStackedWidget
{
	Q_OBJECT

public:
	enum s_directions {
		LEFT2RIGHT,
		RIGHT2LEFT
	};

	explicit StackedHomepage(QWidget *parent = 0);
	~StackedHomepage();

	void addWidget(QWidget *widget);

public Q_SLOTS:
	void moveLeft();
	void moveRight();
	void openFile();

	void setSpeed(int speed);
	void setAnimation(enum QEasingCurve::Type animationType);
	void setWrap(bool wrap);

	void slideInNext();
	void slideInPrev();

protected Q_SLOTS:
	void animationDone();

Q_SIGNALS:
	void animationFinished();

private:
	void slideToIndex(int index);
	void slideInWidget(QWidget* newWidget, enum s_directions direction);

	HomepageControls *s_hc;
	enum QEasingCurve::Type s_animationType;
	int s_speed;
	bool s_wrap;
	bool s_active;
	QPoint s_now;
	int s_current;
	int s_next;
};
}
#endif // STACKED_HOMEPAGE_H
