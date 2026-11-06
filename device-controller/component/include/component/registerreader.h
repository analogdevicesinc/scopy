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

#include <QObject>
#include <cstdint>
#include <qcoro/qcorotask.h>
#include "component/component_export.h"

namespace scopy::component {

// Register-read capability (abstract base): a value addressed by number. Child of a Device
// (IIO debug register access, I2C/SPI register access over a bus dongle, ...), discovered with
// findChild<RegisterReader*>() or the "capability:register-read" tag. addressBits/valueBits
// describe the widths the leaf accepts; 32/32 for libiio.
class COMPONENT_EXPORT RegisterReader : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int addressBits READ addressBits CONSTANT)
	Q_PROPERTY(int valueBits READ valueBits CONSTANT)
public:
	explicit RegisterReader(QObject *parent = nullptr)
		: QObject(parent)
	{}
	~RegisterReader() override = default;

	virtual int addressBits() const { return 32; }
	virtual int valueBits() const { return 32; }

	Q_INVOKABLE virtual QCoro::Task<CommandResponse<uint32_t>> readAsync(uint32_t addr) = 0;

Q_SIGNALS:
	void readSucceeded(uint32_t addr, uint32_t value);
	void readFailed(const scopy::Error &error);
};

} // namespace scopy::component
