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


#ifndef RESISTANCECHNLINFO_H
#define RESISTANCECHNLINFO_H
#include "chnlinfo.h"

#define RPULL_UP 2100
#define ADC_MAX_VALUE 65535
#define MIN_RESISTANCE_VALUE 0
#define MAX_RESISTANCE_VALUE 1000000000
namespace scopy::swiot{
class ResistanceChnlInfo : public ChnlInfo
{
	Q_OBJECT
public:
	explicit ResistanceChnlInfo(QString plotUm = "Ω", QString hwUm = "Ω",
				    iio_channel *iioChnl = nullptr, CommandQueue *cmdQueue = nullptr);
	~ResistanceChnlInfo();

	double convertData(unsigned int data) override;
};
}
#endif // RESISTANCECHNLINFO_H
