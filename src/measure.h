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

#ifndef MEASURE_H
#define MEASURE_H

#include <QList>

namespace adiscope {
	class CrossingDetection;

	class Measure
	{
	public:
		Measure(double *buffer = NULL, size_t length = 0);

		void setDataSource(double *buffer, size_t length);
		void measure();
		double sampleRate();
		void setSampleRate(double);
		unsigned int adcBitCount();
		void setAdcBitCount(unsigned int);
		double crossLevel();
		void setCrossLevel(double);
		double hysteresisSpan();
		void setHysteresisSpan(double);

		double period();
		double frequency();
		double min();
		double max();
		double peakToPeak();
		double mean();
		double rms();
		double rmsAC();
		double low();
		double middle();
		double high();
		double amplitude();
		double positiveOvershoot();
		double negativeOvershoot();
		double riseTime();
		double fallTime();
		double posWidth();
		double negWidth();
		double posDuty();
		double negDuty();

	private:
		bool highLowFromHistogram(double &low, double &high);
		void clearMeasurements();

	private:
		double *m_buffer;
		size_t m_buf_length;
		double m_sample_rate;
		unsigned int m_adc_bit_count;
		double m_cross_level;
		double m_hysteresis_span;

		double m_sum;
		double m_sqr_sum;
		int *m_histogram;
		CrossingDetection *m_cross_detect;

		double m_period;
		double m_frequency;
		double m_rise_time;
		double m_fall_time;
		double m_width_p;
		double m_width_n;
		double m_duty_p;
		double m_duty_n;
		double m_peak_to_peak;
		double m_min;
		double m_max;
		double m_amplitude;
		double m_high;
		double m_middle;
		double m_low;
		double m_overshoot_p;
		double m_overshoot_n;
		double m_mean;
		double m_cycle_mean;
		double m_rms;
		double m_cycle_rms;
		double m_rms_ac;
		double m_cycle_rms_ac;
	};
}

#endif // MEASURE_H
