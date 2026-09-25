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

#include <component/channel.h>
#include <component/attribute.h>
#include <component/backends/iio/iiochannel.h>

#include <float.h>
#include <qmath.h>
#include <iio.h>

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

bool DacUtils::checkDdsChannel(component::Channel *chn)
{
	auto *iioChn = qobject_cast<component::iio::IIOChannel *>(chn);
	if(!iioChn || iioChn->chanType() != IIO_ALTVOLTAGE) {
		return false;
	}
	if(!chn->findChild<component::Attribute *>("frequency")) {
		return false;
	}
	if(!chn->findChild<component::Attribute *>("scale")) {
		return false;
	}
	if(!chn->findChild<component::Attribute *>("phase")) {
		return false;
	}
	return true;
}
