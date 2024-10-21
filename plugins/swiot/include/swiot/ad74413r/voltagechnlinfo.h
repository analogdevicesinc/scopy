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

#ifndef VOLTAGECHNLINFO_H
#define VOLTAGECHNLINFO_H

#include "chnlinfo.h"
#define MIN_VOLTAGE_VALUE -10
#define MAX_VOLTAGE_VALUE 10
namespace scopy::swiot {
class VoltageChnlInfo : public ChnlInfo
{
	Q_OBJECT
public:
	explicit VoltageChnlInfo(QString plotUm = "V", QString hwUm = "mV", iio_channel *iioChnl = nullptr,
				 CommandQueue *cmdQueue = nullptr);
	~VoltageChnlInfo();

	double convertData(unsigned int data) override;
};
} // namespace scopy::swiot

#endif // VOLTAGECHNLINFO_H
