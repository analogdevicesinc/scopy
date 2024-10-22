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

#include "ad74413r/currentchnlinfo.h"

using namespace scopy::swiot;
CurrentChnlInfo::CurrentChnlInfo(QString plotUm, QString hwUm, iio_channel *iioChnl, CommandQueue *cmdQueue)
	: ChnlInfo(plotUm, hwUm, iioChnl, cmdQueue)
{
	m_rangeValues = {MIN_CURRENT_VALUE, MAX_CURRENT_VALUE};
}

CurrentChnlInfo::~CurrentChnlInfo() {}

double CurrentChnlInfo::convertData(unsigned int data)
{
	double convertedData = 0.0;
	double defaultFactor = m_unitOfMeasureFactor.contains(m_hwUm.at(0)) ? m_unitOfMeasureFactor[m_hwUm.at(0)] : 1;
	double newFactor = m_unitOfMeasureFactor.contains(m_plotUm.at(0)) ? m_unitOfMeasureFactor[m_plotUm.at(0)] : 1;
	double factor = defaultFactor / newFactor;
	data <<= 8;
	data = SWAP_UINT32(data);
	data &= 0x0000FFFF;
	convertedData = (data + m_offsetScalePair.first) * m_offsetScalePair.second * factor;
	return convertedData;
}

#include "moc_currentchnlinfo.cpp"
