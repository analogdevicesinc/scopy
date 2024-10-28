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

#ifndef DIGITALCHNLINFO_H
#define DIGITALCHNLINFO_H

#include "chnlinfo.h"
#define MIN_DIGITAL_VALUE -2
#define MAX_DIGITAL_VALUE 2
namespace scopy::swiot {
class DigitalChnlInfo : public ChnlInfo
{
	Q_OBJECT
public:
	explicit DigitalChnlInfo(QString plotUm = "", QString hwUm = "", iio_channel *iioChnl = nullptr,
				 CommandQueue *cmdQueue = nullptr);
	~DigitalChnlInfo();

	double convertData(unsigned int data) override;
};
} // namespace scopy::swiot

#endif // DIGITALCHNLINFO_H
