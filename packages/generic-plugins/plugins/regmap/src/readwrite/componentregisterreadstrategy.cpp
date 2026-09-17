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

#include "componentregisterreadstrategy.hpp"

#include "../logging_categories.h"

#include <component/backends/iio/iioregisterreader.h>

using namespace scopy;
using namespace regmap;

ComponentRegisterReadStrategy::ComponentRegisterReadStrategy(component::iio::IIORegisterReader *reader)
	: m_reader(reader)
{
	// Strip the address space so the UI sees the original register address.
	connect(m_reader, &component::iio::IIORegisterReader::readSucceeded, this,
		[this](uint32_t addr, uint32_t value) { Q_EMIT readDone(addr & ~m_addressSpace, value); });
	connect(m_reader, &component::iio::IIORegisterReader::readFailed, this, [this](const scopy::Error &error) {
		qDebug(CAT_IIO_OPERATION) << "device read error" << error.errorString();
		Q_EMIT readError("device read error");
	});
}

void ComponentRegisterReadStrategy::read(uint32_t address)
{
	if(!m_reader) {
		return;
	}
	m_reader->readAsync(address | m_addressSpace);
}

uint32_t ComponentRegisterReadStrategy::getAddressSpace() const { return m_addressSpace; }

void ComponentRegisterReadStrategy::setAddressSpace(uint32_t newAddressSpace) { m_addressSpace = newAddressSpace; }