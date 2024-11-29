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

#include "ad74413r/chnlinfobuilder.h"

#include "ad74413r/resistancechnlinfo.h"
#include "ad74413r/currentchnlinfo.h"
#include "ad74413r/voltagechnlinfo.h"
#include "ad74413r/digitalchnlinfo.h"

using namespace scopy::swiot;

ChnlInfo *ChnlInfoBuilder::build(iio_channel *iioChnl, QString id, CommandQueue *cmdQueue)
{
	int chnl_type = decodeId(id);
	switch(chnl_type) {
	case VOLTAGE:
		return new VoltageChnlInfo("V", "mV", iioChnl, cmdQueue);
	case CURRENT:
		return new CurrentChnlInfo("A", "mA", iioChnl, cmdQueue);
	case RESISTANCE:
		return new ResistanceChnlInfo("Ω", "Ω", iioChnl, cmdQueue);
	case DIGITAL:
		return new DigitalChnlInfo("unit", "unit", iioChnl, cmdQueue);
	default:
		return nullptr;
	}
}
