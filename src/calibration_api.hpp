#ifndef CALIBRATION_API_HPP
#define CALIBRATION_API_HPP

#include "calibration.hpp"

namespace adiscope {

class Calibration_API : public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(QList<double> adc_offsets READ get_adc_offsets)
	Q_PROPERTY(QList<double> adc_gains READ get_adc_gains)
	Q_PROPERTY(QList<double> dac_offsets READ get_dac_offsets)
	Q_PROPERTY(QList<double> dac_gains READ get_dac_gains)

public:
	explicit Calibration_API(Calibration* calib);
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
	Calibration* calib;
};
} // namespace adiscope

#endif // CALIBRATION_API_HPP
