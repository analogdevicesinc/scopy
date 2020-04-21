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

#ifndef CUSTOM_SWITCH_HPP
#define CUSTOM_SWITCH_HPP

#include "customanimation.h"

#include <QLabel>
#include <QPushButton>
#include <QWidget>

class QShowEvent;

namespace adiscope {
class CustomSwitch : public QPushButton {
	Q_OBJECT

	Q_PROPERTY(int duration_ms MEMBER duration_ms WRITE setDuration);

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
	bool event(QEvent *);

private Q_SLOTS:
	void toggleAnim(bool enabled);
};
} // namespace adiscope

#endif /* CUSTOM_SWITCH_HPP */
