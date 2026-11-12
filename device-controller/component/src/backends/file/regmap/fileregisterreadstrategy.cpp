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

#include "component/backends/file/regmap/fileregisterreadstrategy.h"

#include <QHash>

#include <cerrno>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::component::file;

FileRegisterReadStrategy::FileRegisterReadStrategy(const QString &path, QObject *parent)
	: IRegisterReadStrategy(parent)
	, m_file(path)
{}

QString FileRegisterReadStrategy::path() const { return m_file.path(); }

QCoro::Task<RegisterData> FileRegisterReadStrategy::read(RegisterData data)
{
	// built outside the co_await: GCC fails on a braced temporary inside one
	const QVector<RegisterData> input{data};
	QVector<RegisterData> results = co_await readRange(input);
	co_return results.first();
}

QCoro::Task<QVector<RegisterData>> FileRegisterReadStrategy::readRange(QVector<RegisterData> data)
{
	// one pass over the file: every register of the range shares the stamp
	const auto values = m_file.readAll();
	const QDateTime completedAt = QDateTime::currentDateTime();

	QVector<RegisterData> results;
	results.reserve(data.size());
	if(!values) {
		for(const RegisterData &in : std::as_const(data)) {
			results.append(RegisterData::failed(in.address, values.error(), completedAt));
		}
		co_return results;
	}

	// the first line of an address wins, as for a single read
	QHash<uint32_t, uint32_t> byAddress;
	for(const auto &entry : values.value()) {
		if(!byAddress.contains(entry.first)) {
			byAddress.insert(entry.first, entry.second);
		}
	}
	for(const RegisterData &in : std::as_const(data)) {
		if(byAddress.contains(in.address)) {
			results.append(RegisterData::ok(in.address, byAddress.value(in.address), completedAt));
		} else {
			const Error error{
				-ENOENT,
				QString("no value for address 0x%1 in %2").arg(in.address, 0, 16).arg(m_file.path())};
			results.append(RegisterData::failed(in.address, error, completedAt));
		}
	}
	co_return results;
}
