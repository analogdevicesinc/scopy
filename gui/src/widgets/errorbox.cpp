/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "widgets/errorbox.h"

using namespace scopy;

ErrorBox::ErrorBox(QWidget *parent)
	: QFrame(parent)
{
	setStyleSheet("background-color: transparent; border: 1px solid black;");
	setFixedSize(10, 10);
}

void ErrorBox::changeColor(AvailableColors color)
{
	QString colorString;
	switch(color) {
	case Green:
		colorString = "green";
		break;
	case Yellow:
		colorString = "#808000";
		break;
	case Red:
		colorString = "red";
		break;
	case Transparent:
		colorString = "transparent";
		break;
	}
	setStyleSheet("background-color: " + colorString + "; border: 1px solid black;");
}

#include "moc_errorbox.cpp"
