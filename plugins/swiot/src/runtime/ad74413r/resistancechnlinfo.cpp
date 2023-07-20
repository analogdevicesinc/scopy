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


#include "resistancechnlinfo.h"

using namespace scopy::swiot;

ResistanceChnlInfo::ResistanceChnlInfo(QString plotUm, QString hwUm,
				       iio_channel *iioChnl, CommandQueue *cmdQueue)
	: ChnlInfo(plotUm, hwUm, iioChnl, cmdQueue)
{
	m_rangeValues = {MIN_RESISTANCE_VALUE, MAX_RESISTANCE_VALUE};
}

ResistanceChnlInfo::~ResistanceChnlInfo()
{}

double ResistanceChnlInfo::convertData(unsigned int data)
{
	double convertedData = 0.0;
	data <<= 8;
	data = SWAP_UINT32(data);
	data &= 0x0000FFFF;
	convertedData = ((ADC_MAX_VALUE - data) != 0)
			? ((data * RPULL_UP) / (ADC_MAX_VALUE - data)) : MAX_RESISTANCE_VALUE;
	return convertedData;
}
