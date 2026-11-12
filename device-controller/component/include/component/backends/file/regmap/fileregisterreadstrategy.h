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

#include "component/backends/file/regmap/registervaluesfile.h"
#include "component/regmap/iregisterreadstrategy.h"

#include "component/component_export.h"

namespace scopy::component::file {

// Reads registers from a register values file, as if it were a device.
// A range is one pass over the file with one stamp; an address with no line in the file gets -ENOENT, a file
// that can't be read gives -EIO for every register.
class COMPONENT_EXPORT FileRegisterReadStrategy : public IRegisterReadStrategy
{
	Q_OBJECT
public:
	explicit FileRegisterReadStrategy(const QString &path, QObject *parent = nullptr);

	QString path() const;

	QCoro::Task<RegisterData> read(RegisterData data) override;
	QCoro::Task<QVector<RegisterData>> readRange(QVector<RegisterData> data) override;

private:
	RegisterValuesFile m_file;
};

} // namespace scopy::component::file
