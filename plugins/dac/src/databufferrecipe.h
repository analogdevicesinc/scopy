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

#ifndef DATABUFFERRECIPE_H
#define DATABUFFERRECIPE_H

#include "scopy-dac_export.h"

namespace scopy {
namespace dac {
struct SCOPY_DAC_EXPORT DataBufferRecipe
{
	double scale = 0.0;
	int offset = 0;
	int decimation = 0;
	bool scaled = false;
	double phase = 0.0;
	int frequency = 0;
};
} // namespace dac
} // namespace scopy
#endif // DATABUFFERRECIPE_H
