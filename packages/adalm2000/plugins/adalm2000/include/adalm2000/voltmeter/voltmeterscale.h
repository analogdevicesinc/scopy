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
 */

#pragma once

#include <gui/rangepresetscaler.h>

#include <QVector>

namespace scopy::adalm2000::vm {

inline constexpr int SETTLE_INTERVAL_MS = 3000;

// The M2K analog-in ranges, ascending: RangePresetScaler scans for the first fit.
inline QVector<ScalePreset> presets()
{
	return {
		{-0.1, 0.1, 5, 5},
		{-1.0, 1.0, 5, 5},
		{-5.0, 5.0, 10, 2},
		{-25.0, 25.0, 10, 5},
	};
}

} // namespace scopy::adalm2000::vm
