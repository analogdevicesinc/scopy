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

#include "custom_animation.hpp"

#include <QLabel>
#include <QPushButton>
#include <QWidget>

class QShowEvent;

namespace scopy {
namespace gui {

class CustomSwitch : public QPushButton
{
	Q_OBJECT

	Q_PROPERTY(int m_durationMs MEMBER m_durationMs WRITE setDuration);

public:
	explicit CustomSwitch(QWidget* parent = nullptr);
	~CustomSwitch();

private:
	QLabel m_on, m_off;
	QWidget m_handle;
	CustomAnimation m_anim;
	int m_durationMs;
	bool m_polarity;

	void setDuration(int ms);
	void updateOnOffLabels();

	void showEvent(QShowEvent* event);
	bool event(QEvent*);

private Q_SLOTS:
	void toggleAnim(bool enabled);
};
} // namespace gui
} // namespace scopy

#endif /* CUSTOM_SWITCH_HPP */
