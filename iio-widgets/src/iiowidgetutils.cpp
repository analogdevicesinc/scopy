/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "iiowidgetutils.h"
#include <QDebug>

QString IIOWidgetUtils::comboUiToDataConversionFunction(QString value, QMap<QString, QString> *map)
{
	QMap<QString, QString>::const_iterator i;
	for(i = map->constBegin(); i != map->constEnd(); ++i) {
		if(value == i.value())
			return i.key();
	}

	qWarning() << "No value was found for " << value;

	return "";
}

QString IIOWidgetUtils::comboDataToUiConversionFunction(QString key, QMap<QString, QString> *map)
{
	return map->value(key);
}

#include "moc_iiowidgetutils.cpp"
