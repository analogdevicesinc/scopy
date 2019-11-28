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

	Q_PROPERTY(QList<double> adc_offsets READ get_adc_offsets)
	Q_PROPERTY(QList<double> adc_gains READ get_adc_gains)
	Q_PROPERTY(QList<double> dac_offsets READ get_dac_offsets)
	Q_PROPERTY(QList<double> dac_gains READ get_dac_gains)

public:
	explicit Calibration_API(Calibration *calib);
	QList<double> get_adc_offsets() const;
	QList<double> get_adc_gains() const;
	QList<double> get_dac_offsets() const;
	QList<double> get_dac_gains() const;

	Q_INVOKABLE void setHardwareInCalibMode();
	Q_INVOKABLE void restoreHardwareFromCalibMode();

	Q_INVOKABLE bool resetCalibration();
	Q_INVOKABLE bool calibrateAll();
	Q_INVOKABLE bool setGainMode(int, int);
	Q_INVOKABLE bool setCalibrationMode(int);
	Q_INVOKABLE void dacAOutputDCVolts(int);
	Q_INVOKABLE void dacBOutputDCVolts(int);
	Q_INVOKABLE void dacOutputStop();

	Q_INVOKABLE double devTemp(const QString& devName);

private:
	Calibration *calib;
};
}

#endif // CALIBRATION_API_HPP
