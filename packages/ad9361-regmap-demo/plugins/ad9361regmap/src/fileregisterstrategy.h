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

#ifndef FILEREGISTERSTRATEGY_H
#define FILEREGISTERSTRATEGY_H

#include <QMap>
#include <QSharedPointer>
#include <QString>

#include <regmap/readwrite/iregisterreadstrategy.hpp>
#include <regmap/readwrite/iregisterwritestrategy.hpp>

namespace scopy::ad9361regmap {

// "<address>,<value>" text file (hex, '#' comments) used as a fake register backend.
// The file is re-read on every access so it can be edited while Scopy is running.
class RegisterValuesFile
{
public:
	explicit RegisterValuesFile(const QString &path);

	bool read(uint32_t address, uint32_t &value);
	bool write(uint32_t address, uint32_t value);
	QString path() const;

private:
	bool load();
	bool save() const;

	QString m_path;
	QMap<uint32_t, uint32_t> m_values;
};

class FileRegisterReadStrategy : public regmap::IRegisterReadStrategy
{
	Q_OBJECT
public:
	explicit FileRegisterReadStrategy(QSharedPointer<RegisterValuesFile> file);
	void read(uint32_t address) override;

private:
	QSharedPointer<RegisterValuesFile> m_file;
};

class FileRegisterWriteStrategy : public regmap::IRegisterWriteStrategy
{
	Q_OBJECT
public:
	explicit FileRegisterWriteStrategy(QSharedPointer<RegisterValuesFile> file);
	void write(uint32_t address, uint32_t val) override;

private:
	QSharedPointer<RegisterValuesFile> m_file;
};
} // namespace scopy::ad9361regmap
#endif // FILEREGISTERSTRATEGY_H
