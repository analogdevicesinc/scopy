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

#include "dacutils.h"

#include <float.h>
#include <qmath.h>

using namespace scopy;
using namespace scopy::dac;

#define SCALE_MINUS_INFINITE -91

double DacUtils::dbFullScaleConvert(double scale, bool inverse)
{
	if(inverse) {
		if(scale == 0)
			return -DBL_MAX;
		return (int)((20 * log10(scale)) - 0.5);
	} else {
		if(scale == SCALE_MINUS_INFINITE)
			return 0;
		return pow(10, scale / 20.0);
	}
}

bool DacUtils::checkDdsChannel(iio_channel *chn)
{
	iio_chan_type chnType = iio_channel_get_type(chn);
	if(chnType != IIO_ALTVOLTAGE) {
		return false;
	}
	auto freq = iio_channel_find_attr(chn, "frequency");
	if(!freq) {
		return false;
	}
	auto scale = iio_channel_find_attr(chn, "scale");
	if(!scale) {
		return false;
	}
	auto phase = iio_channel_find_attr(chn, "phase");
	if(!phase) {
		return false;
	}
	return true;
}
