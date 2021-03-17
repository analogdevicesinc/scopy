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

#include <QEasingCurve>
#include <QStackedWidget>
#include <QWidget>

#include <scopy/gui/homepage_controls.hpp>

namespace Ui {
class StackedHomepage;
}

namespace scopy {
namespace gui {

class StackedHomepage : public QStackedWidget
{
	Q_OBJECT

public:
	enum s_directions
	{
		LEFT2RIGHT,
		RIGHT2LEFT
	};

	explicit StackedHomepage(QWidget* parent = 0);
	~StackedHomepage();

	void insertWidget(int pos, QWidget* widget);
	void addWidget(QWidget* widget);
	void removeWidget(QWidget* widget);

	void slideToIndex(int index);

	bool getControlsEnabled() const;
	void setControlsEnabled(bool value);

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

	HomepageControls* m_hc;
	enum QEasingCurve::Type m_animationType;
	int m_speed;
	bool m_wrap;
	bool m_active;
	QPoint m_now;
	int m_current;
	int m_next;
	bool m_controlsEnabled;
};
} // namespace gui
} // namespace scopy

#endif // STACKED_HOMEPAGE_H
