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

#include <readwrite/componentregisterreadstrategy.hpp>

#include "../logging_categories.h"

#include <component/backends/iio/iioregisterreader.h>

#include <cerrno>

using namespace scopy;
using namespace regmap;

ComponentRegisterReadStrategy::ComponentRegisterReadStrategy(component::iio::IIORegisterReader *reader)
	: m_reader(reader)
{}

QCoro::Task<Result<QVector<uint32_t>>> ComponentRegisterReadStrategy::read(uint32_t address, int count)
{
	// copied before awaiting: this strategy can be deleted while a read runs
	QPointer<component::iio::IIORegisterReader> reader = m_reader;
	const uint32_t addressSpace = m_addressSpace;

	QVector<uint32_t> values;
	for(int i = 0; i < count; i++) {
		if(!reader) {
			co_return Unexpected(Error{-ENODEV, "no register reader"});
		}
		auto r = co_await reader->readAsync((address + i) | addressSpace);
		if(!r) {
			qDebug(CAT_IIO_OPERATION) << "device read error" << r.error().errorString();
			co_return Unexpected(r.error());
		}
		values.append(r.value());
	}
	co_return values;
}

uint32_t ComponentRegisterReadStrategy::getAddressSpace() const { return m_addressSpace; }

void ComponentRegisterReadStrategy::setAddressSpace(uint32_t newAddressSpace) { m_addressSpace = newAddressSpace; }