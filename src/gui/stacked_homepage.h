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

#ifndef STACKED_HOMEPAGE_H
#define STACKED_HOMEPAGE_H

#include <QWidget>
#include <QStackedWidget>
#include <QEasingCurve>

#include "gui/homepage_controls.h"

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

	void insertWidget(int pos, QWidget *widget);
	void addWidget(QWidget *widget);
	void removeWidget(QWidget *widget);

	void slideToIndex(int index);

	bool get_controls_enabled() const;
	void set_controls_enabled(bool value);

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
	void moved(int);

private:
	void slideInWidget(QWidget* newWidget, enum s_directions direction);

	HomepageControls *s_hc;
	enum QEasingCurve::Type s_animationType;
	int s_speed;
	bool s_wrap;
	bool s_active;
	QPoint s_now;
	int s_current;
	int s_next;
	bool s_controls_enabled;
};
}
#endif // STACKED_HOMEPAGE_H
