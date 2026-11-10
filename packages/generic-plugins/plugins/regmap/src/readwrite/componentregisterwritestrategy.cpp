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

#include <readwrite/componentregisterwritestrategy.hpp>

#include "../logging_categories.h"

#include <component/backends/iio/iioregisterwriter.h>

#include <cerrno>

using namespace scopy;
using namespace regmap;

ComponentRegisterWriteStrategy::ComponentRegisterWriteStrategy(component::iio::IIORegisterWriter *writer)
	: m_writer(writer)
{}

QCoro::Task<Result<void>> ComponentRegisterWriteStrategy::write(uint32_t address, QVector<uint32_t> values)
{
	// copied before awaiting: this strategy can be deleted while a write runs
	QPointer<component::iio::IIORegisterWriter> writer = m_writer;
	const uint32_t addressSpace = m_addressSpace;

	for(int i = 0; i < values.size(); i++) {
		if(!writer) {
			co_return Unexpected(Error{-ENODEV, "no register writer"});
		}
		auto r = co_await writer->writeAsync((address + i) | addressSpace, values[i]);
		if(!r) {
			qDebug(CAT_IIO_OPERATION) << "device write error" << r.error().errorString();
			co_return Unexpected(r.error());
		}
	}
	co_return Result<void>();
}

uint32_t ComponentRegisterWriteStrategy::getAddressSpace() const { return m_addressSpace; }

void ComponentRegisterWriteStrategy::setAddressSpace(uint32_t newAddressSpace) { m_addressSpace = newAddressSpace; }