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

#pragma once

#include "scopy-core_export.h"
#include "idecoderbackend.h"

#include <memory>

namespace scopy {
namespace decoder {

// Constructs a fresh IDecoderBackend per active decoder instance. Injected
// into DecoderManager so backends are swappable. Must be cheap on the main
// thread; heavy setup belongs in the created backend or its IDecoderCatalog.
class SCOPY_CORE_EXPORT IDecoderBackendFactory
{
public:
	virtual ~IDecoderBackendFactory() = default;

	// Never nullptr on success; on failure, return a backend whose
	// decode() fails with lastError() populated.
	virtual std::unique_ptr<IDecoderBackend> create() = 0;
};

} // namespace decoder
} // namespace scopy
