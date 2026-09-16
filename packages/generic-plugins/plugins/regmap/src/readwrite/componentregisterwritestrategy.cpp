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

#include "componentregisterwritestrategy.hpp"

#include "../logging_categories.h"

#include <component/backends/iio/iioregisterwriter.h>

using namespace scopy;
using namespace regmap;

ComponentRegisterWriteStrategy::ComponentRegisterWriteStrategy(component::iio::IIORegisterWriter *writer)
	: m_writer(writer)
{
	// Strip the address space so the UI sees the original register address.
	connect(m_writer, &component::iio::IIORegisterWriter::writeSucceeded, this,
		[this](uint32_t addr) { Q_EMIT writeSuccess(addr & ~m_addressSpace); });
	connect(m_writer, &component::iio::IIORegisterWriter::writeFailed, this, [this](const scopy::Error &error) {
		qDebug(CAT_IIO_OPERATION) << "device write error" << error.errorString();
		Q_EMIT writeError("device write err");
	});
}

void ComponentRegisterWriteStrategy::write(uint32_t address, uint32_t val)
{
	if(!m_writer) {
		return;
	}
	m_writer->writeAsync(address | m_addressSpace, val);
}

uint32_t ComponentRegisterWriteStrategy::getAddressSpace() const { return m_addressSpace; }

void ComponentRegisterWriteStrategy::setAddressSpace(uint32_t newAddressSpace) { m_addressSpace = newAddressSpace; }