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

#include <climits>

namespace scopy::iio {

// libiio channel types used outside the backends, so that code does not need a libiio header. The values are the
// Linux IIO / hwmon channel type numbers (enum iio_chan_type / enum hwmon_chan_type), identical in libiio v0 and v1.
// Add entries as needed, with their kernel value.
enum ChannelType : int
{
	CHAN_VOLTAGE = 0,
	CHAN_CURRENT = 1,
	CHAN_POWER = 2,
	CHAN_ACCEL = 3,
	CHAN_ANGL_VEL = 4,
	CHAN_MAGN = 5,
	CHAN_TEMP = 9,
	CHAN_ALTVOLTAGE = 15,
	CHAN_PRESSURE = 17,
	CHAN_TYPE_UNKNOWN = INT_MAX,
};

enum HwmonChannelType : int
{
	HWMON_CHAN_VOLTAGE = 0,
	HWMON_CHAN_FAN = 1,
	HWMON_CHAN_TEMP = 3,
	HWMON_CHAN_CURRENT = 4,
	HWMON_CHAN_POWER = 5,
	HWMON_CHAN_ENERGY = 6,
	HWMON_CHAN_HUMIDITY = 7,
	HWMON_CHAN_TYPE_UNKNOWN = CHAN_TYPE_UNKNOWN,
};

} // namespace scopy::iio
