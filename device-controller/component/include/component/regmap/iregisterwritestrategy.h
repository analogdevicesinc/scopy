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

// Writes registers to a source. Same contract as IRegisterReadStrategy; address and value are used from the
// input, and an Ok result carries the value sent. A range is written in its order, carrying on after an error.
class COMPONENT_EXPORT IRegisterWriteStrategy : public QObject
{
	Q_OBJECT
public:
	using QObject::QObject;
	~IRegisterWriteStrategy() override = default;

	virtual QCoro::Task<RegisterData> write(RegisterData data) = 0;
	// the addresses don't have to be contiguous; the order is kept
	virtual QCoro::Task<QVector<RegisterData>> writeRange(QVector<RegisterData> data) = 0;
};

} // namespace scopy::component
