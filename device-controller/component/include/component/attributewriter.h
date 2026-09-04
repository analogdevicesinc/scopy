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
#include <QString>
#include <QUuid>
#include <qcorotask.h>
#include "component/component_export.h"

namespace scopy::component {

// Write capability (abstract base). Child of an Attribute; writeSucceeded is
// wired to the reader's readAsync() for the write-then-read-back settle.
class COMPONENT_EXPORT AttributeWriter : public QObject
{
	Q_OBJECT
public:
	explicit AttributeWriter(QObject *parent = nullptr)
		: QObject(parent)
	{}
	~AttributeWriter() override = default;

	Q_INVOKABLE virtual QCoro::Task<CommandResponse<void>> writeAsync(const QString &value) = 0;

Q_SIGNALS:
	void writeSucceeded();
	void writeFailed(const scopy::Error &error);
};

} // namespace scopy::component
