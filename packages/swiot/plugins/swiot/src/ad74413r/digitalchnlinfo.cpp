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

#include "ad74413r/digitalchnlinfo.h"

using namespace scopy::swiot;

DigitalChnlInfo::DigitalChnlInfo(QString plotUm, QString hwUm, iio_channel *iioChnl, CommandQueue *cmdQueue)
	: ChnlInfo(plotUm, hwUm, iioChnl, cmdQueue)
{
	m_rangeValues = {MIN_DIGITAL_VALUE, MAX_DIGITAL_VALUE};
}

DigitalChnlInfo::~DigitalChnlInfo() {}

double DigitalChnlInfo::convertData(unsigned int data)
{
	double convertedData = 0.0;
	data <<= 8;
	data = SWAP_UINT32(data);
	data &= 0x0000FFFF;
	convertedData = (data + m_offsetScalePair.first) * m_offsetScalePair.second;
	return convertedData;
}

#include "moc_digitalchnlinfo.cpp"
