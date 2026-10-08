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
#include "component/component_export.h"

namespace scopy::component::iio {

// Builds a libiio-backed Context: instantiates the libiio backend linked into this
// build (SCOPY_LIBIIO_VERSION selects v0 or v1 at compile time), opens the context
// handle, attaches a serial executor, and runs the IIO component builder.
class COMPONENT_EXPORT IIOContextFactory : public ContextFactory
{
public:
	IIOContextFactory() = default;

	Context *create(const QString &uri, const int maxThreads) override;
};

} // namespace scopy::component::iio
