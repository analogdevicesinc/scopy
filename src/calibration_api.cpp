#include "calibration_api.hpp"

namespace adiscope {

Calibration_API::Calibration_API(Calibration* calib)
	: ApiObject()
	, calib(calib)
{}

QList<double> Calibration_API::get_adc_offsets() const
{
	QList<double> offsets;

	offsets.push_back(static_cast<double>(calib->adcOffsetChannel0()));
	offsets.push_back(static_cast<double>(calib->adcOffsetChannel1()));

	return offsets;
}

QList<double> Calibration_API::get_adc_gains() const
{
	QList<double> gains = {calib->adcGainChannel0(), calib->adcGainChannel1()};

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
	QList<double> gains = {calib->dacAvlsb(), calib->dacBvlsb()};

	return gains;
}

bool Calibration_API::calibrateAll() { return calib->calibrateAll(); }

bool Calibration_API::resetCalibration() { return calib->resetCalibration(); }

bool Calibration_API::setGainMode(int ch, int mode) { return calib->setGainMode(ch, mode); }

bool Calibration_API::setCalibrationMode(int mode) { return calib->setCalibrationMode(mode); }

void Calibration_API::setHardwareInCalibMode() { calib->setHardwareInCalibMode(); }

void Calibration_API::dacAOutputDCVolts(int value) { calib->dacAOutputDCVolts(value); }

void Calibration_API::dacBOutputDCVolts(int value) { calib->dacBOutputDCVolts(value); }

void Calibration_API::dacOutputStop() { calib->dacOutputStop(); }

void Calibration_API::restoreHardwareFromCalibMode() { calib->restoreHardwareFromCalibMode(); }

double Calibration_API::devTemp(const QString& devName) { return calib->getIioDevTemp(devName); }

} // namespace adiscope
