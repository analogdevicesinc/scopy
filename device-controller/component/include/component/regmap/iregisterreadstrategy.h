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

#include "component/regmap/registerdata.h"

#include <QObject>
#include <QVector>

#include <qcoro/qcorotask.h>
#include "component/component_export.h"

namespace scopy::component {

// Reads registers from a source (a device, a file, ...). The contract of every read strategy:
// 1. one RegisterData per input, in the same order, with the same address; only address is used from the input
// 2. it maps its source to RegisterData: status, value (null on Error), error and completedAt
// 3. registers that come back together share one completedAt; one call per register, one stamp each
// 4. a range carries on after an error: every register is attempted and gets its own status
// 5. it can be deleted while a call runs, so it never touches `this` after a co_await
// The device controller creates and owns the strategies; tools only hold them through QPointer.
class COMPONENT_EXPORT IRegisterReadStrategy : public QObject
{
	Q_OBJECT
public:
	using QObject::QObject;
	~IRegisterReadStrategy() override = default;

	virtual QCoro::Task<RegisterData> read(RegisterData data) = 0;
	// the addresses don't have to be contiguous
	virtual QCoro::Task<QVector<RegisterData>> readRange(QVector<RegisterData> data) = 0;
};

} // namespace scopy::component
