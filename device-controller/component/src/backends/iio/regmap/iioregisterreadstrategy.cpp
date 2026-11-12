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

#include "component/backends/iio/regmap/iioregisterreadstrategy.h"

#include "component/backends/iio/iioregisterreader.h"

#include <cerrno>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::component::iio;

namespace {
// takes copies only: the strategy can be deleted while the read runs
QCoro::Task<RegisterData> readOne(QPointer<IIORegisterReader> reader, uint32_t addressSpace, uint32_t address)
{
	if(!reader) {
		co_return RegisterData::failed(address, Error{-ENODEV, "no register reader"},
					       QDateTime::currentDateTime());
	}
	auto r = co_await reader->readAsync(address | addressSpace);
	const QDateTime completedAt = QDateTime::currentDateTime();
	if(!r) {
		co_return RegisterData::failed(address, r.error(), completedAt);
	}
	co_return RegisterData::ok(address, r.value(), completedAt);
}
} // namespace

IIORegisterReadStrategy::IIORegisterReadStrategy(IIORegisterReader *reader, uint32_t addressSpace, QObject *parent)
	: IRegisterReadStrategy(parent)
	, m_reader(reader)
	, m_addressSpace(addressSpace)
{}

uint32_t IIORegisterReadStrategy::addressSpace() const { return m_addressSpace; }

QCoro::Task<RegisterData> IIORegisterReadStrategy::read(RegisterData data)
{
	return readOne(m_reader, m_addressSpace, data.address);
}

QCoro::Task<QVector<RegisterData>> IIORegisterReadStrategy::readRange(QVector<RegisterData> data)
{
	QPointer<IIORegisterReader> reader = m_reader;
	const uint32_t addressSpace = m_addressSpace;

	QVector<RegisterData> results;
	results.reserve(data.size());
	for(const RegisterData &in : std::as_const(data)) {
		results.append(co_await readOne(reader, addressSpace, in.address));
	}
	co_return results;
}
