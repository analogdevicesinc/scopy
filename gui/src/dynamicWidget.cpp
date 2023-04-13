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

#include "dynamicWidget.h"

#include <QStyle>
#include <QVariant>
#include <QWidget>

void scopy::setDynamicProperty(QWidget *widget,
		const char *property, bool set)
{
	widget->setProperty(property, QVariant(set));
	widget->style()->unpolish(widget);
	widget->style()->polish(widget);
}

bool scopy::getDynamicProperty(QWidget *widget,
				  const char *property)
{
	QVariant val = widget->property(property);
	if (val.canConvert<bool>()) {
		return val.toBool();
	}
	return false;
}
