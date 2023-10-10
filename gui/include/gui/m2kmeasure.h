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

#ifndef M2KMEASURE_H
#define M2KMEASURE_H

#include "scopy-gui_export.h"

#include <QList>
#include <QString>

#include <memory>

namespace scopy {
class M2kCrossingDetection;

class SCOPY_GUI_EXPORT M2kMeasurementData
{
public:
	enum unitTypes
	{
		METRIC,
		TIME,
		PERCENTAGE,
		DIMENSIONLESS,
		DECIBELS,
		DECIBELS_TO_CARRIER
	};

	enum axisType
	{
		HORIZONTAL,
		VERTICAL,
		HORIZONTAL_F,
		VERTICAL_F
	};

	M2kMeasurementData(const QString &name, axisType axis, const QString &unit = "", int channel = -1);

	QString name() const;
	double value() const;
	void setValue(double value);
	bool measured() const;
	void setMeasured(bool state);
	bool enabled() const;
	void setEnabled(bool en);
	QString unit() const;
	enum unitTypes unitType() const;
	int channel() const;
	void setChannel(int);
	enum axisType axis() const;

private:
	QString m_name;
	double m_value;
	bool m_measured;
	bool m_enabled;
	QString m_unit;
	enum unitTypes m_unitType;
	int m_channel;
	enum axisType m_axis;
};

class SCOPY_GUI_EXPORT M2kMeasure
{
public:
	enum defaultMeasurements
	{
		PERIOD = 0,
		FREQUENCY,
		MIN,
		MAX,
		PEAK_PEAK,
		MEAN,
		CYCLE_MEAN,
		RMS,
		CYCLE_RMS,
		AC_RMS,
		AREA,
		CYCLE_AREA,
		LOW,
		HIGH,
		AMPLITUDE,
		MIDDLE,
		P_OVER,
		N_OVER,
		RISE,
		FALL,
		P_WIDTH,
		N_WIDTH,
		P_DUTY,
		N_DUTY,
		DEFAULT_MEASUREMENT_COUNT
	};

	enum defaultSpectralMeasurements
	{
		NOISE_FLOOR = 0,
		SINAD,
		SNR,
		THD,
		THDN,
		SFDR,
		DEFAULT_SPECTRAL_MEASUREMENT_COUNT
	};

	M2kMeasure(int channel, double *buffer = NULL, size_t length = 0,
		   const std::function<double(unsigned int, double, bool)> &conversion = nullptr,
		   bool isTimeDomain = true);

	void setDataSource(double *buffer, size_t length);
	void measure();

	void measureTimeDomain();
	void measureSpectral();

	double sampleRate();
	void setSampleRate(double);
	unsigned int adcBitCount();
	void setAdcBitCount(unsigned int);
	double crossLevel();
	void setCrossLevel(double);
	double hysteresisSpan();
	void setHysteresisSpan(double);
	int channel() const;
	void setChannel(int);
	void setStartIndex(int);
	void setEndIndex(int);
	void setGatingEnabled(bool);

	void setHarmonicNumber(int harmonics_number);
	void setMask(std::vector<int> mask);

	QList<std::shared_ptr<M2kMeasurementData>> measurments();
	std::shared_ptr<M2kMeasurementData> measurement(int id);
	int activeMeasurementsCount() const;

	void setConversionFunction(const std::function<double(unsigned int, double, bool)> &fp);

	std::vector<int> LoadMaskfromFile(std::string file_name);

private:
	bool highLowFromHistogram(double &low, double &high, double min, double max);
	void clearMeasurements();

private:
	int m_channel;
	double *m_buffer;
	ssize_t m_buf_length;
	double m_sample_rate;
	unsigned int m_adc_bit_count;
	double m_cross_level;
	double m_hysteresis_span;
	int m_startIndex;
	int m_endIndex;
	int m_gatingEnabled;
	int *m_histogram;
	M2kCrossingDetection *m_cross_detect;

	int m_harmonics_number;
	std::vector<int> m_mask;
	bool m_isTimeDomain;

	QList<std::shared_ptr<M2kMeasurementData>> m_measurements;
	std::function<double(unsigned int, double, bool)> m_conversion_function;
};

class SCOPY_GUI_EXPORT M2kStatistic
{
public:
	M2kStatistic();

	void pushNewData(double data);
	void clear();

	double average() const;
	double min() const;
	double max() const;
	double numPushedData() const;

private:
	double m_sum;
	double m_min;
	double m_max;
	double m_dataCount;
	double m_average;
};
} // namespace scopy

#endif // M2KMEASURE_H
