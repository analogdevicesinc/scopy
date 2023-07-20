/*
 * Copyright (c) 2023 Analog Devices Inc.
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


#ifndef CHNLINFOBUILDER_H
#define CHNLINFOBUILDER_H

#include "src/runtime/ad74413r/chnlinfo.h"

namespace scopy::swiot {
class ChnlInfoBuilder
{
public:
	enum ChnlInfoType {
		VOLTAGE		= 0,
		CURRENT		= 1,
		RESISTANCE	= 2
	};

	static int decodeId(QString function)
	{
		if (function.compare("v") == 0) {
			return VOLTAGE;
		} else if (function.compare("c") == 0) {
			return CURRENT;
		} else if (function.compare("r") == 0) {
			return RESISTANCE;
		} else {
			return -1;
		}
	}
	static ChnlInfo* build(iio_channel *iioChnl, QString id, CommandQueue *cmdQueue);

};
}

#endif // CHNLINFOBUILDER_H
