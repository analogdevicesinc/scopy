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

#include "measure.h"
#include "adc_sample_conv.hpp"
#include <qmath.h>
#include <QDebug>

using namespace adiscope;

namespace adiscope {
	class CrossPoint
	{
		public:
		CrossPoint(float value, size_t bufIndex, bool onRising):
			m_value(value),
			m_bufIdx(bufIndex),
			m_onRising(onRising)

		{
		}

	public:
		float m_value;
		size_t m_bufIdx;
		bool m_onRising;
	};

	class HystLevelCross
	{
	public:
		enum crossEvents {
			NO_CROSS = 0,
			POS_CROSS_LOW,
			POS_CROSS_HIGH,
			POS_CROSS_FULL,
			NEG_CROSS_LOW,
			NEG_CROSS_HIGH,
			NEG_CROSS_FULL,
		};

		HystLevelCross() :
			m_low_trhold_crossed(false),
			m_high_trhold_crossed(false),
			m_is_between_trholds(false)
		{
		}

		virtual inline bool updateState(enum crossEvents crsEvent) = 0;

		static inline enum crossEvents
			get_crossing_type(double samp, double prevSamp,
					double low_trhold, double high_trhold)
		{
			enum crossEvents cross_type = NO_CROSS;

			if (samp > prevSamp) {
				if (prevSamp <= low_trhold && samp >= low_trhold)
					cross_type = POS_CROSS_LOW;
				if (prevSamp <= high_trhold && samp >= high_trhold)
					if (cross_type == POS_CROSS_LOW)
						cross_type = POS_CROSS_FULL;
					else
						cross_type = POS_CROSS_HIGH;
			} else if (samp < prevSamp) {
				if (prevSamp >= low_trhold && samp <= low_trhold)
					cross_type = NEG_CROSS_LOW;
				if (prevSamp >= high_trhold && samp <= high_trhold)
					if (cross_type == NEG_CROSS_LOW)
						cross_type = NEG_CROSS_FULL;
					else
						cross_type = NEG_CROSS_HIGH;
			}

			return cross_type;
		}

		void resetState()
		{
			m_low_trhold_crossed = false;
			m_high_trhold_crossed = false;
			m_is_between_trholds = false;
		}

	protected:
		bool m_low_trhold_crossed;
		bool m_high_trhold_crossed;
		bool m_is_between_trholds;
	};

	class HystLevelPosCross: public HystLevelCross
	{
	public:
		HystLevelPosCross() :
			HystLevelCross()
		{
		}

		inline bool updateState(enum crossEvents crsEvent)
		{
			bool level_crossed = false;

			switch (crsEvent) {
			case POS_CROSS_LOW:
				m_is_between_trholds = true;
				break;
			case POS_CROSS_HIGH:
				if (m_is_between_trholds) {
					level_crossed = true;
					m_is_between_trholds = false;
				}
				break;
			case POS_CROSS_FULL:
				level_crossed = true;
				break;
			case NEG_CROSS_LOW:
				m_is_between_trholds = false;
			default:
				break;
			}

			return level_crossed;
		}
	};

	class HystLevelNegCross: public HystLevelCross
	{
	public:
		HystLevelNegCross() :
			HystLevelCross()
		{
		}

		inline bool updateState(enum crossEvents crsEvent)
		{
			bool level_crossed = false;

			switch (crsEvent) {
			case NEG_CROSS_HIGH:
				m_is_between_trholds = true;
				break;
			case NEG_CROSS_LOW:
				if (m_is_between_trholds) {
					level_crossed = true;
					m_is_between_trholds = false;
				}
				break;
			case NEG_CROSS_FULL:
				level_crossed = true;
				break;
			case POS_CROSS_HIGH:
				m_is_between_trholds = false;
			default:
				break;
			}

			return level_crossed;
		}
	};

	class CrossingDetection
	{
	public:
		CrossingDetection(double level, double hysteresis_span):
			m_posCrossFound(false),
			m_negCrossFound(false),
			m_level(level),
			m_hysteresis_span(hysteresis_span),
			m_low_level(level - hysteresis_span / 2),
			m_high_level(level + hysteresis_span / 2),
			m_negSampleFound(false)
		{
		}

		double level()
		{
			return m_level;
		}

		void setLevel(double level)
		{
			if (m_level != level) {
				m_level = level;
				m_low_level = level - m_hysteresis_span / 2;
				m_high_level = level + m_hysteresis_span / 2;
			}
		}

		double hysteresisSpan()
		{
			return m_hysteresis_span;
		}

		void setHysteresisSpan(double span)
		{
			if (m_hysteresis_span != span) {
				m_hysteresis_span = span;
				m_low_level = m_level - span / 2;
				m_high_level = m_level + span / 2;
			}
		}

		QList<CrossPoint> detectedCrossings()
		{
			return m_detectedCrossings;
		}

		inline void crossDetectStep(double *data, size_t i)
		{
			auto cross_type = HystLevelCross::get_crossing_type(data[i],
						data[i - 1], m_low_level, m_high_level);

			if (cross_type != HystLevelCross::NO_CROSS) {
				if (!m_posCrossFound) {
					m_crossed = m_posCross.updateState(cross_type);
					if (m_crossed) {
						m_posCrossFound = true;
						m_negCrossFound = false;
						m_negCross.resetState();
						m_detectedCrossings.push_back(
							CrossPoint(data[i], i, true));
					}
				}
				if (!m_negCrossFound) {
					m_crossed = m_negCross.updateState(cross_type);
					if (m_crossed) {
						m_negCrossFound = true;
						m_posCrossFound = false;
						m_posCross.resetState();
						size_t idx;
						if (m_negSampleFound) {
							idx = m_negSampleIdx;
							m_negSampleFound = false;
						} else {
							idx = i - 1;
						}
						m_detectedCrossings.push_back(
							CrossPoint(data[idx], idx, false));
					}
					if (cross_type == HystLevelCross::NEG_CROSS_HIGH) {
						m_negSampleIdx = i;
						m_negSampleFound = true;
					}
				}
			}
		}

	private:
		HystLevelPosCross m_posCross;
		HystLevelNegCross m_negCross;

		bool m_posCrossFound;
		bool m_negCrossFound;
		bool m_crossed;

		double m_level;
		double m_hysteresis_span;
		double m_low_level;
		double m_high_level;

		size_t m_negSampleIdx; // Stores the sample at a negative high threshold cross
		bool m_negSampleFound;

		QList<CrossPoint> m_detectedCrossings;
	};
}

Measure::Measure(double *buffer, size_t length):
	m_buffer(buffer),
	m_buf_length(length),
	m_sample_rate(1.0),
	m_adc_bit_count(0),
	m_cross_level(0),
	m_hysteresis_span(0),
	m_sum(0),
	m_sqr_sum(0),
	m_period(0),
	m_frequency(0),
	m_rise_time(0),
	m_fall_time(0),
	m_width_p(0),
	m_width_n(0),
	m_duty_p(0),
	m_duty_n(0),
	m_peak_to_peak(0),
	m_min(0),
	m_max(0),
	m_amplitude(0),
	m_high(0),
	m_middle(0),
	m_low(0),
	m_overshoot_p(0),
	m_overshoot_n(0),
	m_mean(0),
	m_cycle_mean(0),
	m_rms(0),
	m_cycle_rms(0),
	m_rms_ac(0),
	m_cycle_rms_ac(0)
{
}

bool Measure::highLowFromHistogram(double &low, double &high)
{
	bool success = false;
	int *hist = m_histogram;
	int adc_span = 1 << m_adc_bit_count;
	int hlf_scale = adc_span / 2;

	int minRaw = adc_sample_conv::convVoltsToSample(m_min) + hlf_scale;
	int maxRaw = adc_sample_conv::convVoltsToSample(m_max) + hlf_scale;
	int middleRaw = minRaw + (maxRaw - minRaw)  / 2;

	auto lowIt = std::max_element(hist + minRaw, hist + middleRaw + 1);
	int lowRaw = std::distance(hist, lowIt);

	auto highIt = std::max_element(hist + middleRaw, hist + maxRaw + 1);
	int highRaw = std::distance(hist, highIt);

	/* Use histogram results if High and Low settling levels can be
	   clearly identified (weight of a level should be 5 times
	   greater than a peak weight - there probably is a better method) */

	if (hist[lowRaw] / 5.0 >= hist[minRaw] &&
		hist[highRaw] / 5.0 >= hist[maxRaw]) {
		low = adc_sample_conv::convSampleToVolts(lowRaw - hlf_scale);
		high = adc_sample_conv::convSampleToVolts(highRaw - hlf_scale);
		success = true;
	}

	return success;
}

void Measure::clearMeasurements()
{
	 m_period = 0;
	 m_frequency = 0;
	 m_rise_time = 0;
	 m_fall_time = 0;
	 m_width_p = 0;
	 m_width_n = 0;
	 m_duty_p = 0;
	 m_duty_n = 0;
	 m_peak_to_peak = 0;
	 m_min = 0;
	 m_max = 0;
	 m_amplitude = 0;
	 m_high = 0;
	 m_middle = 0;
	 m_low = 0;
	 m_overshoot_p = 0;
	 m_overshoot_n = 0;
	 m_mean = 0;
	 m_cycle_mean = 0;
	 m_rms = 0;
	 m_cycle_rms = 0;
	 m_rms_ac = 0;
	 m_cycle_rms_ac = 0;
}

void Measure::setDataSource(double *buffer, size_t length)
{
	m_buffer = buffer;
	m_buf_length = length;
}

void Measure::measure()
{
	clearMeasurements();

	if (!m_buffer || m_buf_length == 0)
		return;

	// Cache buffer address, length, ADC bit count
	double *data = m_buffer;
	size_t data_length = m_buf_length;
	int adc_span = 1 << m_adc_bit_count;
	int hlf_scale = adc_span / 2;
	bool using_histogram_method = (adc_span > 1);

	m_max = data[0];
	m_min = data[0];
	m_sum = data[0];
	m_sqr_sum = data[0] * data[0];
	m_cross_detect = new CrossingDetection(m_cross_level, m_hysteresis_span);
	if (using_histogram_method)
		m_histogram = new int[adc_span]{};

	for (size_t i = 1; i < data_length; i++) {

		// Find level crossings (period detection)
		m_cross_detect->crossDetectStep(data, i);

		// Min
		if (data[i] < m_min)
			m_min = data[i];

		// Max
		if (data[i] > m_max)
			m_max = data[i];

		// Sum of values
		m_sum += data[i];

		// Sum of the squares of values
		m_sqr_sum += data[i] * data[i];

		// Build histogram
		if (using_histogram_method) {
			int raw = hlf_scale + (int)adiscope::adc_sample_conv::
						convVoltsToSample(data[i]);
			if (raw >= 0 && raw  < adc_span)
				m_histogram[raw] += 1;
		}
	}

	// Peak-to-Peak
	m_peak_to_peak = qAbs(m_max - m_min);

	// Mean
	m_mean = m_sum / data_length;

	// RMS
	m_rms = sqrt(m_sqr_sum / data_length);

	// AC RMS
	m_rms_ac = sqrt((m_sqr_sum - 2 * m_mean * m_sum +
		data_length *  m_mean * m_mean) / data_length);


	double low = m_min;
	double high = m_max;

	// Try to use Histogram method
	if (using_histogram_method)
		highLowFromHistogram(low, high);

	// Low, High, Middle, Amplitude, Overshoot positive/negative
	m_low = low;
	m_high = high;
	m_middle = m_low + (m_high - m_low) / 2.0;
	m_amplitude = m_high - m_low;
	m_overshoot_p = (m_max - m_high) / m_amplitude * 100;
	m_overshoot_n = (m_low - m_min) / m_amplitude * 100;

	if (m_histogram != NULL) {
		delete[] m_histogram;
		m_histogram = NULL;
	}

	// Find Period / Frequency
	QList<CrossPoint> periodPoints = m_cross_detect->detectedCrossings();
	int n = periodPoints.size();
	if (n > 2) {
		double sample_period;
		double first_hlf_cycl = 0;
		double secnd_hlf_cycl = 0;
		int j = 0;
		int k = 0;

		for (int i = 0; i < n - 1; i++) {
			size_t diff = periodPoints[i + 1].m_bufIdx -
					periodPoints[i].m_bufIdx;
			if (i % 2) {
				secnd_hlf_cycl += diff;
				j++;
				}
			else {
				first_hlf_cycl += diff;
				k++;
				}
		}

		sample_period = first_hlf_cycl / (n / 2) +
				secnd_hlf_cycl / ((n + 1) / 2 - 1);
		m_period = sample_period * (1 / m_sample_rate);
		m_frequency = 1 / m_period;

		// Find level crossings (10%, 50%, 90%)
		double lowRef = m_low + (0.1 * m_amplitude);
		double midRef = m_low + (0.5 * m_amplitude);
		double highRef = m_low + (0.9 * m_amplitude);

		CrossingDetection cdLow(lowRef, 0.2);
		CrossingDetection cdMid(midRef, 0.2);
		CrossingDetection cdHigh(highRef, 0.2);

		size_t period_start = periodPoints[0].m_bufIdx;
		size_t period_end = periodPoints[2].m_bufIdx;
		size_t length = period_end - period_start + 1;

		qDebug() << "start:" << period_start;
		qDebug() << "end:" << period_end;
		qDebug() << "lowRef:" << lowRef;
		qDebug() << "midRef:" << midRef;
		qDebug() << "highRef:" << highRef;

		for (size_t i = period_start + 1; i <= period_end; i++) {
			cdLow.crossDetectStep(data, i);
			cdMid.crossDetectStep(data, i);
			cdHigh.crossDetectStep(data, i);
		}

		QList<CrossPoint> lowPoints = cdLow.detectedCrossings();
		QList<CrossPoint> midPoints = cdMid.detectedCrossings();
		QList<CrossPoint> highPoints = cdHigh.detectedCrossings();

		qDebug() << "Low Crossings";
		for (int i = 0; i < lowPoints.size(); i++)
			qDebug() << "val:" << lowPoints[i].m_value << " idx:" << lowPoints[i].m_bufIdx << " rise:" << lowPoints[i].m_onRising;

		qDebug() << "Middle Crossings";
		for (int i = 0; i < midPoints.size(); i++)
			qDebug() << "val:" << midPoints[i].m_value << " idx:" << midPoints[i].m_bufIdx << " rise:" << midPoints[i].m_onRising;

		qDebug() << "High Crossings";
		for (int i = 0; i < highPoints.size(); i++)
			qDebug() << "val:" << highPoints[i].m_value << " idx:" << highPoints[i].m_bufIdx << " rise:" << highPoints[i].m_onRising;

		if (lowPoints.size() < 2 || midPoints.size() < 2 || highPoints.size() < 2) {
			qDebug() << "Unable to find 2 transitions for each of the 10%, 50%, 90% levels";
		} else {
			if (!lowPoints[0].m_onRising)
				lowPoints.swap(0, 1);
			if (!midPoints[0].m_onRising)
				midPoints.swap(0, 1);
			if (!highPoints[0].m_onRising)
				highPoints.swap(0, 1);

			// Rise Time
			long long rise = (long long)(highPoints[0].m_bufIdx -
					lowPoints[0].m_bufIdx);
			if (rise < 0)
				rise += length;
			m_rise_time = rise / m_sample_rate;

			// Fall Time
			long long fall = (long long)(lowPoints[1].m_bufIdx -
					highPoints[1].m_bufIdx);
			if (fall < 0)
				fall += length;
			m_fall_time = fall / m_sample_rate;

			// Positive Width
			long long posWidth = (long long)(midPoints[1].m_bufIdx -
					midPoints[0].m_bufIdx);
			if (posWidth < 0)
				posWidth += length;

			m_width_p = posWidth / m_sample_rate;

			// Negative Width
			m_width_n = m_period - m_width_p;

			// Positive Duty
			m_duty_p = m_width_p / m_period * 100;

			// Negative Duty
			m_duty_n = m_width_n / m_period * 100;
		}
	}

	delete m_cross_detect;
	m_cross_detect = NULL;

}

double Measure::sampleRate()
{
	return m_sample_rate;
}

void Measure::setSampleRate(double value)
{
	m_sample_rate = value;
}

unsigned int Measure::adcBitCount()
{
	return m_adc_bit_count;
}

void Measure::setAdcBitCount(unsigned int val)
{
	m_adc_bit_count = val;
}

double Measure::crossLevel()
{
	return m_cross_level;
}

void Measure::setCrossLevel(double value)
{
	m_cross_level = value;
}

double Measure::hysteresisSpan()
{
	return m_hysteresis_span;
}

void Measure::setHysteresisSpan(double value)
{
	m_hysteresis_span = value;
}

double Measure::period()
{
	return m_period;
}

double Measure::frequency()
{
	return m_frequency;
}

double Measure::min()
{
	return m_min;
}

double Measure::max()
{
	return m_max;
}

double Measure::peakToPeak()
{
	return m_peak_to_peak;
}

double Measure::mean()
{
	return m_mean;
}

double Measure::rms()
{
	return m_rms;
}

double Measure::rmsAC()
{
	return m_rms_ac;
}

double Measure::low()
{
	return m_low;
}

double Measure::middle()
{
	return m_middle;
}

double Measure::high()
{
	return m_high;
}

double Measure::amplitude()
{
	return m_amplitude;
}

double Measure::positiveOvershoot()
{
	return m_overshoot_p;
}

double Measure::negativeOvershoot()
{
	return m_overshoot_n;
}

double Measure::riseTime()
{
	return m_rise_time;
}

double Measure::fallTime()
{
	return m_fall_time;
}

double Measure::posWidth()
{
	return m_width_p;
}

double Measure::negWidth()
{
	return m_width_n;
}

double Measure::posDuty()
{
	return m_duty_p;
}

double Measure::negDuty()
{
	return m_duty_n;
}
