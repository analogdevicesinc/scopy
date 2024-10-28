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

#include "registermodel.hpp"

#include "bitfield/bitfieldmodel.hpp"

using namespace scopy::regmap;

RegisterModel::RegisterModel(QString name, uint32_t address, QString description, bool exists, int width, QString notes,
			     QVector<BitFieldModel *> *bitFields, RegisterMapTemplate *registerMapTemplate)
	: name(name)
	, address(address)
	, description(description)
	, exists(exists)
	, width(width)
	, notes(notes)
	, bitFields(bitFields)
	, m_registerMapTemaplate(registerMapTemplate)
{}

QString RegisterModel::getName() const { return name; }

uint32_t RegisterModel::getAddress() const { return address; }

QString RegisterModel::getDescription() const { return description; }

bool RegisterModel::getExists() const { return exists; }

int RegisterModel::getWidth() const { return width; }

QString RegisterModel::getNotes() const { return notes; }

QVector<BitFieldModel *> *RegisterModel::getBitFields() const { return bitFields; }

RegisterMapTemplate *RegisterModel::registerMapTemaplate() const { return m_registerMapTemaplate; }

void RegisterModel::setRegisterMapTemaplate(RegisterMapTemplate *newRegisterMapTemaplate)
{
	m_registerMapTemaplate = newRegisterMapTemaplate;
}
