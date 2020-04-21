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

#ifndef SCOPY_CUSTOM_PUSH_BUTTON_HPP
#define SCOPY_CUSTOM_PUSH_BUTTON_HPP

#include <QPushButton>

class QMouseEvent;

namespace adiscope {
/* The CustomPushButton class behaves exactly like QPushButton. The only
 * difference is, when it's part of a button group and is the active
 * button, it is possible to disactivate it by clicking on it. */

class CustomPushButton : public QPushButton {
	Q_OBJECT

public:
	explicit CustomPushButton(QWidget *parent = Q_NULLPTR);
	~CustomPushButton();

protected:
	void mouseReleaseEvent(QMouseEvent *event);

public Q_SLOTS:
	void setChecked(bool checked);
};
} // namespace adiscope

#endif /* SCOPY_CUSTOM_PUSH_BUTTON_HPP */
