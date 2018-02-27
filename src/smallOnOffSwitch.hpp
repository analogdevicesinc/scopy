/*
 * Copyright 2017 Analog Devices, Inc.
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

#ifndef SMALL_ON_OFF_SWITCH_HPP
#define SMALL_ON_OFF_SWITCH_HPP

#include <QColor>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QWidget>

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
		QWidget handle;
		QPropertyAnimation anim;
		QPropertyAnimation color_anim;
		QString stylesheet;
		int duration_ms;
		bool show_icon;

		void setDuration(int ms);
		void setHandleColor(const QColor& color);

		void showEvent(QShowEvent *event);
		void paintEvent(QPaintEvent *);

	private Q_SLOTS:
		void toggleAnim(bool enabled);
	};
}

#endif /* SMALL_ON_OFF_SWITCH_HPP */
