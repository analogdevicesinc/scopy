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
#include "calibration_api.hpp"

namespace adiscope {

Calibration_API::Calibration_API(Calibration *calib) : ApiObject(),
	calib(calib)
{
}

QList<double> Calibration_API::get_adc_offsets() const
{
	QList<double> offsets;

	offsets.push_back(static_cast<double>(calib->adcOffsetChannel0()));
	offsets.push_back(static_cast<double>(calib->adcOffsetChannel1()));

	return offsets;
}

QList<double> Calibration_API::get_adc_gains() const
{
	QList<double> gains = { calib->adcGainChannel0(),
		calib->adcGainChannel1() };

	return gains;
}

QList<double> Calibration_API::get_dac_offsets() const
{
	QList<double> offsets;

	offsets.push_back(static_cast<double>(calib->dacAoffset()));
	offsets.push_back(static_cast<double>(calib->dacBoffset()));

	return offsets;
}

QList<double> Calibration_API::get_dac_gains() const
{
	QList<double> gains = { calib->dacAvlsb(),
		calib->dacBvlsb() };

	return gains;
}

bool Calibration_API::calibrateAll()
{
	return calib->calibrateAll();
}

bool Calibration_API::resetCalibration()
{
	return calib->resetCalibration();
}

bool Calibration_API::setGainMode(int ch, int mode)
{
	return calib->setGainMode(ch, mode);
}

bool Calibration_API::setCalibrationMode(int mode)
{
	return calib->setCalibrationMode(mode);
}

void Calibration_API::setHardwareInCalibMode()
{
	calib->setHardwareInCalibMode();
}

void Calibration_API::dacAOutputDCVolts(int value)
{
	calib->dacAOutputDCVolts(value);
}

void Calibration_API::dacBOutputDCVolts(int value)
{
	calib->dacBOutputDCVolts(value);
}

void Calibration_API::dacOutputStop()
{
	calib->dacOutputStop();
}

void Calibration_API::restoreHardwareFromCalibMode()
{
	calib->restoreHardwareFromCalibMode();
}

double Calibration_API::devTemp(const QString& devName)
{
	return calib->getIioDevTemp(devName);
}


}
