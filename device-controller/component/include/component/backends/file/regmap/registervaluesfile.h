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

#pragma once

#include "core/result.h"

#include <QPair>
#include <QString>
#include <QVector>

#include <cstdint>
#include "component/component_export.h"

namespace scopy::component::file {

// "<address>,<value>" text file (hex, "0x" optional, '#' comments) that stands in for a device.
// It is read again on every access, so it can be edited while Scopy runs.
class COMPONENT_EXPORT RegisterValuesFile
{
public:
	explicit RegisterValuesFile(const QString &path);

	// -ENOENT if the file has no line for the address, -EIO if it can't be read
	scopy::Result<uint32_t> read(uint32_t address) const;
	// rewrites the line of the address, or appends one
	scopy::Result<void> write(uint32_t address, uint32_t value);
	// the same for every value, in order, with one read and one write of the file
	scopy::Result<void> write(const QVector<QPair<uint32_t, uint32_t>> &values);
	// every valid line, in file order; malformed lines are skipped
	scopy::Result<QVector<QPair<uint32_t, uint32_t>>> readAll() const;

	QString path() const;

private:
	QString m_path;
};

} // namespace scopy::component::file
