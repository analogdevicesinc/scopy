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

#ifndef IREGISTERREADSTRATEGY_HPP
#define IREGISTERREADSTRATEGY_HPP

#include "../scopy-regmap_export.h"

#include <QObject>
#include <QVector>

#include <core/result.h>
#include <qcoro/qcorotask.h>

namespace scopy::regmap {
// Each call returns its own result: one value per address, or the first error.
// A strategy can be deleted while a call runs, so it must not touch `this` after a co_await.
class IRegisterReadStrategy : public QObject
{
	Q_OBJECT
public:
	// reads count registers from address on
	virtual QCoro::Task<scopy::Result<QVector<uint32_t>>> read(uint32_t address, int count = 1) = 0;
};
} // namespace scopy::regmap
#endif // IREGISTERREADSTRATEGY_HPP
