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

#include "component/backends/iio/regmap/iioregisterwritestrategy.h"

#include "component/backends/iio/iioregisterwriter.h"

#include <cerrno>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::component::iio;

namespace {
// takes copies only: the strategy can be deleted while the write runs
QCoro::Task<RegisterData> writeOne(QPointer<IIORegisterWriter> writer, uint32_t addressSpace, RegisterData data)
{
	if(!data.value) {
		co_return RegisterData::failed(data.address, Error{-EINVAL, "no value to write"},
					       QDateTime::currentDateTime());
	}
	if(!writer) {
		co_return RegisterData::failed(data.address, Error{-ENODEV, "no register writer"},
					       QDateTime::currentDateTime());
	}
	auto r = co_await writer->writeAsync(data.address | addressSpace, *data.value);
	const QDateTime completedAt = QDateTime::currentDateTime();
	if(!r) {
		co_return RegisterData::failed(data.address, r.error(), completedAt);
	}
	co_return RegisterData::ok(data.address, *data.value, completedAt);
}
} // namespace

IIORegisterWriteStrategy::IIORegisterWriteStrategy(IIORegisterWriter *writer, uint32_t addressSpace, QObject *parent)
	: IRegisterWriteStrategy(parent)
	, m_writer(writer)
	, m_addressSpace(addressSpace)
{}

uint32_t IIORegisterWriteStrategy::addressSpace() const { return m_addressSpace; }

QCoro::Task<RegisterData> IIORegisterWriteStrategy::write(RegisterData data)
{
	return writeOne(m_writer, m_addressSpace, data);
}

QCoro::Task<QVector<RegisterData>> IIORegisterWriteStrategy::writeRange(QVector<RegisterData> data)
{
	QPointer<IIORegisterWriter> writer = m_writer;
	const uint32_t addressSpace = m_addressSpace;

	QVector<RegisterData> results;
	results.reserve(data.size());
	// in order, and every register is attempted
	for(const RegisterData &in : std::as_const(data)) {
		results.append(co_await writeOne(writer, addressSpace, in));
	}
	co_return results;
}
