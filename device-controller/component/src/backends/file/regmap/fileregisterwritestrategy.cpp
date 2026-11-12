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

#include "component/backends/file/regmap/fileregisterwritestrategy.h"

#include <cerrno>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::component::file;

FileRegisterWriteStrategy::FileRegisterWriteStrategy(const QString &path, QObject *parent)
	: IRegisterWriteStrategy(parent)
	, m_file(path)
{}

QString FileRegisterWriteStrategy::path() const { return m_file.path(); }

QCoro::Task<RegisterData> FileRegisterWriteStrategy::write(RegisterData data)
{
	// built outside the co_await: GCC fails on a braced temporary inside one
	const QVector<RegisterData> input{data};
	QVector<RegisterData> results = co_await writeRange(input);
	co_return results.first();
}

QCoro::Task<QVector<RegisterData>> FileRegisterWriteStrategy::writeRange(QVector<RegisterData> data)
{
	QVector<QPair<uint32_t, uint32_t>> values;
	for(const RegisterData &in : std::as_const(data)) {
		if(in.value) {
			values.append({in.address, *in.value});
		}
	}
	// one pass over the file, in order: every register of the range shares the stamp
	const Result<void> written = values.isEmpty() ? Result<void>() : m_file.write(values);
	const QDateTime completedAt = QDateTime::currentDateTime();

	QVector<RegisterData> results;
	results.reserve(data.size());
	for(const RegisterData &in : std::as_const(data)) {
		if(!in.value) {
			results.append(
				RegisterData::failed(in.address, Error{-EINVAL, "no value to write"}, completedAt));
		} else if(!written) {
			results.append(RegisterData::failed(in.address, written.error(), completedAt));
		} else {
			results.append(RegisterData::ok(in.address, *in.value, completedAt));
		}
	}
	co_return results;
}
