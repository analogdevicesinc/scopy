/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef IREGISTERWRITESTRATEGY_HPP
#define IREGISTERWRITESTRATEGY_HPP

#include "../scopy-regmap_export.h"

#include <QObject>
#include <QVector>

#include <core/result.h>
#include <qcoro/qcorotask.h>

namespace scopy::regmap {
// Each call returns its own result: nothing on success, or the first error.
// A strategy can be deleted while a call runs, so it must not touch `this` after a co_await.
class IRegisterWriteStrategy : public QObject
{
	Q_OBJECT
public:
	// writes values to address, address + 1, ...
	virtual QCoro::Task<scopy::Result<void>> write(uint32_t address, QVector<uint32_t> values) = 0;
};
} // namespace scopy::regmap
#endif // IREGISTERWRITESTRATEGY_HPP
