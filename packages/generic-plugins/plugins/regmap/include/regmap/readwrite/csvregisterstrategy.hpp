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

#ifndef CSVREGISTERSTRATEGY_HPP
#define CSVREGISTERSTRATEGY_HPP

#include "../scopy-regmap_export.h"
#include "iregisterreadstrategy.hpp"
#include "iregisterwritestrategy.hpp"

#include <QSharedPointer>
#include <QString>

namespace scopy::regmap {

// "<address>,<value>" text file (hex, '#' comments) used as a register backend.
// It is read again on every access, so it can be edited while Scopy runs.
class SCOPY_REGMAP_EXPORT CsvRegisterFile
{
public:
	explicit CsvRegisterFile(const QString &path);

	// false if the file can't be read or has no line for the address
	bool read(uint32_t address, uint32_t &value) const;
	// rewrites the line of the address, or appends one
	bool write(uint32_t address, uint32_t value);
	QString path() const;

private:
	QString m_path;
};

class SCOPY_REGMAP_EXPORT CsvRegisterReadStrategy : public IRegisterReadStrategy
{
	Q_OBJECT
public:
	explicit CsvRegisterReadStrategy(QSharedPointer<CsvRegisterFile> file);
	void read(uint32_t address) override;

private:
	QSharedPointer<CsvRegisterFile> m_file;
};

class SCOPY_REGMAP_EXPORT CsvRegisterWriteStrategy : public IRegisterWriteStrategy
{
	Q_OBJECT
public:
	explicit CsvRegisterWriteStrategy(QSharedPointer<CsvRegisterFile> file);
	void write(uint32_t address, uint32_t val) override;

private:
	QSharedPointer<CsvRegisterFile> m_file;
};
} // namespace scopy::regmap
#endif // CSVREGISTERSTRATEGY_HPP
