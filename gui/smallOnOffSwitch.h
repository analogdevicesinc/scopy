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

#ifndef SMALL_ON_OFF_SWITCH_HPP
#define SMALL_ON_OFF_SWITCH_HPP

#include <QColor>
#include "gui/customanimation.h"
#include <QPushButton>
#include <QWidget>
#include <QLabel>

class QShowEvent;

namespace adiscope {
	class SmallOnOffSwitch : public QPushButton
	{
		Q_OBJECT

		Q_PROPERTY(int duration_ms MEMBER duration_ms
				WRITE setDuration);

		Q_PROPERTY(QColor color_start MEMBER color_start);
		Q_PROPERTY(QColor color_end MEMBER color_end);
		Q_PROPERTY(QColor color MEMBER color WRITE setHandleColor);

	public:
		explicit SmallOnOffSwitch(QWidget *parent = nullptr);
		~SmallOnOffSwitch();

	private:
		QColor color_start, color_end, color;
		QLabel on, off;
		QWidget handle;
		CustomAnimation anim;
		CustomAnimation color_anim;
		QString stylesheet;
		int duration_ms;
		bool show_icon;
		bool bothValid;

		void setDuration(int ms);
		void setHandleColor(const QColor& color);
		void updateOnOffLabels();

		bool event(QEvent *e);
		void showEvent(QShowEvent *event);
		void paintEvent(QPaintEvent *);

	private Q_SLOTS:
		void toggleAnim(bool enabled);
	Q_SIGNALS:
		void animationDone();
	};
}

#endif /* SMALL_ON_OFF_SWITCH_HPP */
