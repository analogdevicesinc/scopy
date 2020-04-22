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

#ifndef LCD_NUMBER_HPP
#define LCD_NUMBER_HPP

#include <QLCDNumber>

namespace adiscope {
class LcdNumber : public QLCDNumber
{
	Q_OBJECT

public:
	explicit LcdNumber(QWidget* parent = 0, unsigned precision = 3);
	~LcdNumber() {}

public Q_SLOTS:
	void display(double num);

private:
	unsigned precision;
};
} // namespace adiscope

#endif /* LCD_NUMBER_HPP */
