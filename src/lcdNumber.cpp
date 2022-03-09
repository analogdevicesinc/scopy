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

#include "lcdNumber.hpp"

#include <iostream>
#include <locale>
#include <sstream>

using namespace adiscope;

LcdNumber::LcdNumber(QWidget *parent, unsigned precision) :
	QLCDNumber(parent), precision(precision)
{
	display(0.0);
}

void LcdNumber::setPrecision(unsigned precision)
{
	this->precision = precision;

}

unsigned LcdNumber::getPrecision()
{
	return precision;
}

void LcdNumber::display(double num)
{
	std::stringstream str;

	str.imbue(std::locale("C"));
	str.precision(this->precision);
	str.setf(std::ios::fixed, std::ios::floatfield);

	str << num;

	QLCDNumber::display(QString(str.str().c_str()));
}
