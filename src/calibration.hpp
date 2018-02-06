/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CALIBRATION_HPP
#define CALIBRATION_HPP

#include "apiObject.hpp"

#include <cstdint>
#include <cstdlib>
#include <string>
#include <memory>

extern "C" {
	struct iio_context;
	struct iio_device;
	struct iio_channel;
	struct iio_buffer;
}

namespace adiscope {

class M2kAdc;
class M2kDac;
class Calibration_API;

class Calibration
{
	friend class Calibration_API;

public:
	enum calibration_mode {
		ADC_REF1,
		ADC_REF2,
		ADC_GND,
		DAC,
		NONE
	};

        enum gain_mode {
                LOW,
                HIGH
        };

	Calibration(struct iio_context *ctx, QJSEngine *engine,
		    std::shared_ptr<M2kAdc> adc = nullptr,
		    std::shared_ptr<M2kDac> dac_a = nullptr,
		    std::shared_ptr<M2kDac> dac_b = nullptr);
	~Calibration();

	bool initialize();
	bool isInitialized() const;

	void setHardwareInCalibMode();
	void restoreHardwareFromCalibMode();

	bool calibrateAll();
	bool calibrateADCoffset();
	bool calibrateADCgain();
	bool calibrateDACoffset();
	bool calibrateDACgain();
	void cancelCalibration();

	int adcOffsetChannel0() const;
	int adcOffsetChannel1() const;
	int dacAoffset() const;
	int dacBoffset() const;
	double adcGainChannel0() const;
	double adcGainChannel1() const;
	double dacAvlsb() const;
	double dacBvlsb() const;

	bool resetCalibration();
	void updateCorrections();

	double getIioDevTemp(const QString& devName) const;

	static void setChannelEnableState(struct iio_channel *chn, bool en);
	static double average(int16_t *data, size_t numElements);
	static float convSampleToVolts(float sample, float correctionGain = 1);
	static float convVoltsToSample(float sample, float correctionGain = 1);

	bool setCalibrationMode(int);
	bool setGainMode(int ch, int);
	void dacAOutputDCVolts(int16_t volts);
	void dacBOutputDCVolts(int16_t volts);
	void dacOutputStop();

	int getAdc_ch0_offset() const;
	void setAdc_ch0_offset(int adc_ch0_offset);

	int getAdc_ch1_offset() const;
	void setAdc_ch1_offset(int adc_ch1_offset);

	int getDac_a_ch_offset() const;
	void setDac_a_ch_offset(int dac_a_ch_offset);

	int getDac_b_ch_offset() const;
	void setDac_b_ch_offset(int dac_b_ch_offset);

	double getAdc_ch0_gain() const;
	void setAdc_ch0_gain(double adc_ch0_gain);

	double getAdc_ch1_gain() const;
	void setAdc_ch1_gain(double adc_ch1_gain);

	double getDac_a_ch_vlsb() const;
	void setDac_a_ch_vlsb(double dac_a_ch_vlsb);

	double getDac_b_ch_vlsb() const;
	void setDac_b_ch_vlsb(double dac_b_ch_vlsb);

private:
	bool adc_data_capture(int16_t *dataCh0, int16_t *dataCh1,
			      size_t num_sampl_per_chn);
	bool fine_tune(size_t span, int16_t centerVal0, int16_t centerVal1,
		size_t num_samples);

	void dacOutputDC(struct iio_device *dac, struct iio_channel *channel,
		struct iio_buffer** buffer, size_t value);
	void dacAOutputDC(int16_t value);
	void dacBOutputDC(int16_t value);
	void configHwSamplerate();

	ApiObject *m_api;
	bool m_cancel;

	std::shared_ptr<M2kAdc> m2k_adc;
	std::shared_ptr<M2kDac> m2k_dac_a;
	std::shared_ptr<M2kDac> m2k_dac_b;

	struct iio_context *m_ctx;
	struct iio_device *m_m2k_adc;
	struct iio_device *m_m2k_dac_a;
	struct iio_device *m_m2k_dac_b;
	struct iio_device *m2k_ad5625;
	struct iio_device *m_m2k_fabric;

	struct iio_channel *m_adc_channel0;
	struct iio_channel *m_adc_channel1;

	struct iio_channel *m_dac_a_channel;
	struct iio_channel *m_dac_b_channel;

	struct iio_channel *m_dac_a_fabric;
	struct iio_channel *m_dac_b_fabric;

	struct iio_channel *m_ad5625_channel0;
	struct iio_channel *m_ad5625_channel1;
	struct iio_channel *m_ad5625_channel2;
	struct iio_channel *m_ad5625_channel3;

	struct iio_buffer *m_dac_a_buffer;
	struct iio_buffer *m_dac_b_buffer;

	int m_adc_ch0_offset;
	int m_adc_ch1_offset;
	int m_dac_a_ch_offset;
	int m_dac_b_ch_offset;
	double m_adc_ch0_gain;
	double m_adc_ch1_gain;
	double m_dac_a_ch_vlsb;
	double m_dac_b_ch_vlsb;

	std::string m_trigger0_mode;
	std::string m_trigger1_mode;
	double adc_sampl_freq;
	double adc_oversampl;
	double dac_a_sampl_freq;
	double dac_a_oversampl;
	double dac_b_sampl_freq;
	double dac_b_oversampl;

	bool m_initialized;
	int m_calibration_mode;
};

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

} // namespace adiscope

#endif /* CALIBRATION_HPP */
