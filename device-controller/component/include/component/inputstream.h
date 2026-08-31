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

#include "component/stream.h"
#include "component/streamformat.h"

#include <QUuid>

namespace scopy::component {

class InputStream : public Stream
{
	Q_OBJECT
public:
	explicit InputStream(QObject *parent = nullptr)
		: Stream(parent)
	{}

	Q_INVOKABLE virtual QCoro::Task<CommandResponse<void>> refillAsync() = 0;

	// Read-only view of the captured buffer; valid after a successful refill().
	virtual const StreamFormat &readFormat() const = 0;

Q_SIGNALS:
	void refillSucceeded();
	void refillFailed(const scopy::Error &error);
};

} // namespace scopy::component
