/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "registermaptemplatepostprocessor.hpp"

#include "register/bitfield/bitfieldmodel.hpp"
#include "register/registermodel.hpp"
#include "registermaptemplate.hpp"

#include <algorithm>

using namespace scopy::regmap;

void RegisterMapTemplatePostProcessor::apply(RegisterMapTemplate *registerMapTemplate,
					     const RegisterMapTemplateOptions &options)
{
	registerMapTemplate->setBitsPerRow(options.bitsPerRow);
	for(RegisterModel *reg : *registerMapTemplate->getRegisterList()) {
		swapNameAndDescription(reg, options);
		if(options.fillReservedBitFields) {
			fillReservedBitFields(reg);
		}
	}
}

void RegisterMapTemplatePostProcessor::swapNameAndDescription(RegisterModel *reg,
							      const RegisterMapTemplateOptions &options)
{
	if(options.useRegisterDescriptionAsName) {
		const QString name = reg->getName();
		reg->setName(reg->getDescription());
		reg->setDescription(name);
	}

	if(options.useBitFieldDescriptionAsName && reg->getBitFields()) {
		for(BitFieldModel *bf : *reg->getBitFields()) {
			const QString name = bf->getName();
			bf->setName(bf->getDescription());
			bf->setDescription(name);
		}
	}
}

// Adds a "Reserved" bitfield for every gap between the described bitfields and for the unused top bits. Kept
// identical to the original XML reader: bits below the first bitfield are not filled, and the list is only sorted
// when bits are missing.
void RegisterMapTemplatePostProcessor::fillReservedBitFields(RegisterModel *reg)
{
	QVector<BitFieldModel *> *bitFieldsList = reg->getBitFields();
	if(!bitFieldsList || bitFieldsList->empty()) {
		return;
	}

	const int regWidth = reg->getWidth();
	int numberOfBits = 0;
	for(const BitFieldModel *bf : std::as_const(*bitFieldsList)) {
		numberOfBits += bf->getWidth();
	}
	if(regWidth <= numberOfBits) {
		return;
	}

	std::sort(bitFieldsList->begin(), bitFieldsList->end(), [](const BitFieldModel *bf1, const BitFieldModel *bf2) {
		return bf1->getRegOffset() < bf2->getRegOffset();
	});

	QVector<BitFieldModel *> *bitFieldsListWithReserved = new QVector<BitFieldModel *>();
	bitFieldsListWithReserved->push_back(bitFieldsList->at(0));
	for(int i = 1; i < bitFieldsList->length(); i++) {
		int bf1 = bitFieldsList->at(i - 1)->getRegOffset() + bitFieldsList->at(i - 1)->getWidth();
		int bf2 = bitFieldsList->at(i)->getRegOffset();
		if(bf1 != bf2) {
			bitFieldsListWithReserved->push_back(
				new BitFieldModel("Bit " + QString::number(bf2 - 1) + ":" + QString::number(bf1),
						  bf2 - bf1, bf1, "Reserved", nullptr));
			numberOfBits += bf2 - bf1;
		}
		bitFieldsListWithReserved->push_back(bitFieldsList->at(i));
	}

	if(regWidth > numberOfBits) {
		bitFieldsListWithReserved->push_back(
			new BitFieldModel("Bit " + QString::number(regWidth - 1) + ":" + QString::number(numberOfBits),
					  regWidth - numberOfBits, numberOfBits, "Reserved", nullptr));
	}

	reg->setBitFields(bitFieldsListWithReserved);
	delete bitFieldsList;
}
