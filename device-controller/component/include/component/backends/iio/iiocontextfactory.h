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

#include "component/contextfactory.h"
#include "iioutil/handles.h"

namespace scopy::component::iio {

// Builds a libiio-backed Context: loads the backend plugin for its fixed
// LibiioVersion, opens the context handle, attaches a serial executor, and
// runs the IIO component builder. One factory instance per version.
class IIOContextFactory : public ContextFactory
{
public:
	explicit IIOContextFactory(scopy::iio::LibiioVersion version)
		: m_version(version)
	{}

	Context *create(const QString &uri) override;

private:
	scopy::iio::LibiioVersion m_version;
};

} // namespace scopy::component::iio
