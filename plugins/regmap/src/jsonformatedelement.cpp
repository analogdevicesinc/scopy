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

#include "jsonformatedelement.hpp"

using namespace scopy::regmap;

JsonFormatedElement::JsonFormatedElement(QString fileName, QList<QString> *compatibleDevices, bool isAxiCompatible,
					 bool useRegisterDescriptionAsName, bool useBifieldDescriptionAsName,
					 int bitsPerRow)
	: fileName(fileName)
	, compatibleDevices(compatibleDevices)
	, isAxiCompatible(isAxiCompatible)
	, useRegisterDescriptionAsName(useRegisterDescriptionAsName)
	, useBifieldDescriptionAsName(useBifieldDescriptionAsName)
	, bitsPerRow(bitsPerRow)
{}

QString JsonFormatedElement::getFileName() const { return fileName; }

QList<QString> *JsonFormatedElement::getCompatibleDevices() const { return compatibleDevices; }

void JsonFormatedElement::addCompatibleDevice(QString device) { compatibleDevices->push_back(device); }

bool JsonFormatedElement::getIsAxiCompatible() const { return isAxiCompatible; }

bool JsonFormatedElement::getUseBifieldNameAsDescription() const { return useBifieldDescriptionAsName; }

QString JsonFormatedElement::toString()
{
	return QString("Filename : " + fileName + " Compatible Dev: " + compatibleDevices->length() +
		       " isAxiCompatible :" + isAxiCompatible + " useRegisterDescriptionAsName: " +
		       useRegisterDescriptionAsName + " useBifieldDescriptionAsName: " + useBifieldDescriptionAsName);
}

int JsonFormatedElement::getBitsPerRow() const { return bitsPerRow; }

bool JsonFormatedElement::getUseRegisterNameAsDescription() const { return useRegisterDescriptionAsName; }
