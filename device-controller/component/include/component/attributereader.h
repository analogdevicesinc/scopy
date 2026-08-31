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
#include <QUuid>
#include <qcorotask.h>

namespace scopy::component {

// Read capability (abstract base). Child of an Attribute; readSucceeded is wired
// to Attribute::setCachedValue.
class AttributeReader : public QObject
{
	Q_OBJECT
public:
	explicit AttributeReader(QObject *parent = nullptr)
		: QObject(parent)
	{}
	~AttributeReader() override = default;

	Q_INVOKABLE virtual QCoro::Task<CommandResponse<QByteArray>> readAsync() = 0;

Q_SIGNALS:
	void readSucceeded(scopy::Result<QByteArray> &result);
	void readFailed(const scopy::Error &error);
};

} // namespace scopy::component

Q_DECLARE_METATYPE(scopy::Result<QByteArray>)
