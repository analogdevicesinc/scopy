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

#include "registermaptemplate.hpp"

#include "register/bitfield/bitfieldmodel.hpp"
#include "register/registermodel.hpp"
#include "search.hpp"
#include "xmlfilemanager.hpp"

#include <QMap>

using namespace scopy;
using namespace regmap;

RegisterMapTemplate::RegisterMapTemplate(QObject *parent)
	: QObject{parent}
{
	registerList = new QMap<uint32_t, RegisterModel *>();
}

RegisterMapTemplate::~RegisterMapTemplate() { delete registerList; }

QMap<uint32_t, RegisterModel *> *RegisterMapTemplate::getRegisterList() const { return registerList; }

RegisterModel *RegisterMapTemplate::getRegisterTemplate(uint32_t address)
{
	if(registerList->contains(address)) {
		return registerList->value(address);
	}
	return getDefaultTemplate(address);
}

void RegisterMapTemplate::setRegisterList(QMap<uint32_t, RegisterModel *> *newRegisterList)
{
	registerList = newRegisterList;
}

RegisterModel *RegisterMapTemplate::getDefaultTemplate(uint32_t address)
{
	QVector<BitFieldModel *> *bitFieldsList = new QVector<BitFieldModel *>();

	for(int i = 0; i < 8; ++i) {
		bitFieldsList->push_back(new BitFieldModel("Bit " + QString::number(i), 1, 8 - i, ""));
	}

	return new RegisterModel("Register Name", address, "Register Description", true, 8, "Reigster Notes",
				 bitFieldsList, this);
}

int RegisterMapTemplate::bitsPerRow() const { return m_bitsPerRow; }

void RegisterMapTemplate::setBitsPerRow(int newBitsPerRow) { m_bitsPerRow = newBitsPerRow; }
