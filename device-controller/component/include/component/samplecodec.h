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

#include <QObject>

namespace scopy::component {

// Per-channel single-sample codec: pure computation, no executor. Converts one
// sample between hardware and host representation.
class SampleCodec : public QObject
{
	Q_OBJECT
public:
	explicit SampleCodec(QObject *parent = nullptr)
		: QObject(parent)
	{}
	~SampleCodec() override = default;

	virtual void convert(void *dst, const void *src) const = 0;
	virtual void convertInverse(void *dst, const void *src) const = 0;
};

} // namespace scopy::component
