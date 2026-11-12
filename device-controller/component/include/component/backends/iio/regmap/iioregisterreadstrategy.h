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

#include "component/regmap/iregisterreadstrategy.h"

#include <QPointer>

#include "component/component_export.h"

namespace scopy::component::iio {

class IIORegisterReader;

// Reads through the device's IIORegisterReader. The address space is fixed at creation and OR-ed into the device
// call only; the results keep the template address.
// A range is one readAsync per register, in order: each register is stamped when it answers, and the range
// carries on after an error.
class COMPONENT_EXPORT IIORegisterReadStrategy : public IRegisterReadStrategy
{
	Q_OBJECT
public:
	IIORegisterReadStrategy(IIORegisterReader *reader, uint32_t addressSpace, QObject *parent = nullptr);

	uint32_t addressSpace() const;

	QCoro::Task<RegisterData> read(RegisterData data) override;
	QCoro::Task<QVector<RegisterData>> readRange(QVector<RegisterData> data) override;

private:
	QPointer<IIORegisterReader> m_reader;
	const uint32_t m_addressSpace;
};

} // namespace scopy::component::iio
