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

#include "search.hpp"

#include "register/bitfield/bitfieldmodel.hpp"
#include "register/registermodel.hpp"

using namespace scopy;
using namespace regmap;

Search::Search(QObject *parent)
	: QObject{parent}
{}

QList<uint32_t> Search::searchForRegisters(QMap<uint32_t, RegisterModel *> *registers, QString searchParam)
{
	if(searchParam.isEmpty()) {
		return registers->keys();
	}

	QList<uint32_t> result;
	QMap<uint32_t, RegisterModel *>::iterator mapIterator;
	for(mapIterator = registers->begin(); mapIterator != registers->end(); ++mapIterator) {
		QString address = QString::number(mapIterator.key(), 16);
		if(address.contains(searchParam) || mapIterator.value()->getName().toLower().contains(searchParam) ||
		   mapIterator.value()->getDescription().toLower().contains(searchParam)) {
			result.push_back(mapIterator.key());
		} else {
			for(int i = 0; i < mapIterator.value()->getBitFields()->size(); ++i) {
				if(mapIterator.value()->getBitFields()->at(i)->getName().toLower().contains(
					   searchParam) ||
				   mapIterator.value()->getBitFields()->at(i)->getDescription().toLower().contains(
					   searchParam)) {

					result.push_back(mapIterator.key());
					break;
				}
			}
		}
	}
	return result;
}
