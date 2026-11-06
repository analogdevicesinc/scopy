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

// Register-write capability (abstract base); see RegisterReader. Tag "capability:register-write".
class COMPONENT_EXPORT RegisterWriter : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int addressBits READ addressBits CONSTANT)
	Q_PROPERTY(int valueBits READ valueBits CONSTANT)
public:
	explicit RegisterWriter(QObject *parent = nullptr)
		: QObject(parent)
	{}
	~RegisterWriter() override = default;

	virtual int addressBits() const { return 32; }
	virtual int valueBits() const { return 32; }

	Q_INVOKABLE virtual QCoro::Task<CommandResponse<void>> writeAsync(uint32_t addr, uint32_t value) = 0;

Q_SIGNALS:
	void writeSucceeded(uint32_t addr);
	void writeFailed(const scopy::Error &error);
};

} // namespace scopy::component
