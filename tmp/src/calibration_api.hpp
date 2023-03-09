/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CALIBRATION_API_HPP
#define CALIBRATION_API_HPP

#include "calibration.hpp"

namespace adiscope {

class Calibration_API: public ApiObject
{
	Q_OBJECT

public:
	explicit Calibration_API(Calibration *calib);
	Q_INVOKABLE bool resetCalibration();
	Q_INVOKABLE bool calibrateAll();
	Q_INVOKABLE bool calibrateAdc();
	Q_INVOKABLE bool calibrateDac();

	Q_INVOKABLE double devTemp(const QString& devName);

private:
	Calibration *calib;
};
}

#endif // CALIBRATION_API_HPP
