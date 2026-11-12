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

#include "component/regmap/iregisterwritestrategy.h"

#include <QPointer>

#include "component/component_export.h"

namespace scopy::component::iio {

class IIORegisterWriter;

// Writes through the device's IIORegisterWriter, with a fixed address space (see IIORegisterReadStrategy).
// A range is one writeAsync per register, in order, carrying on after an error.
class COMPONENT_EXPORT IIORegisterWriteStrategy : public IRegisterWriteStrategy
{
	Q_OBJECT
public:
	IIORegisterWriteStrategy(IIORegisterWriter *writer, uint32_t addressSpace, QObject *parent = nullptr);

	uint32_t addressSpace() const;

	QCoro::Task<RegisterData> write(RegisterData data) override;
	QCoro::Task<QVector<RegisterData>> writeRange(QVector<RegisterData> data) override;

private:
	QPointer<IIORegisterWriter> m_writer;
	const uint32_t m_addressSpace;
};

} // namespace scopy::component::iio
