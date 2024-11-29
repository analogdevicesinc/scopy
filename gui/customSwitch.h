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

#ifndef CUSTOM_SWITCH_HPP
#define CUSTOM_SWITCH_HPP

#include <QLabel>
#include "gui/customanimation.h"
#include <QPushButton>
#include <QWidget>

class QShowEvent;

namespace adiscope {
	class CustomSwitch : public QPushButton
	{
		Q_OBJECT

		Q_PROPERTY(int duration_ms MEMBER duration_ms
				WRITE setDuration);

	public:
		explicit CustomSwitch(QWidget *parent = nullptr);
		~CustomSwitch();

	private:
		QLabel on, off;
		QWidget handle;
		CustomAnimation anim;
		int duration_ms;
		bool polarity;

		void setDuration(int ms);
		void updateOnOffLabels();

		void showEvent(QShowEvent *event);
		bool event(QEvent*);

	private Q_SLOTS:
		void toggleAnim(bool enabled);
	};
}

#endif /* CUSTOM_SWITCH_HPP */
