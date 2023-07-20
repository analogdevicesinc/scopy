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


#ifndef CURRENTCHNLINFO_H
#define CURRENTCHNLINFO_H

#include "chnlinfo.h"
#define MIN_CURRENT_VALUE -25
#define MAX_CURRENT_VALUE 25
namespace scopy::swiot{
class CurrentChnlInfo : public ChnlInfo
{
	Q_OBJECT
public:
	explicit CurrentChnlInfo(QString plotUm = "mA", QString hwUm = "mA",
				 iio_channel *iioChnl = nullptr, CommandQueue *cmdQueue = nullptr);
	~CurrentChnlInfo();

	double convertData(unsigned int data) override;
};
}

#endif // CURRENTCHNLINFO_H
