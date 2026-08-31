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

namespace scopy::iio {

struct DataFormat
{
	unsigned int length;
	unsigned int bits;
	unsigned int shift;
	bool is_signed;
	bool is_fully_defined;
	bool is_be;
	bool with_scale;
	double scale;
	unsigned int repeat;
	double offset; // v1 only; v0 backend sets 0.0
};

} // namespace scopy::iio
