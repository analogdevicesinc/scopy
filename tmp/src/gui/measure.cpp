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

#include "measure.h"
#include <fstream>
#include <cmath>
#include "adc_sample_conv.hpp"
#include <qmath.h>
#include <QDebug>
#include <QObject>
#include <string>
#include <fstream>

using namespace adiscope;

namespace adiscope {
	class CrossPoint
	{
		public:
		CrossPoint(float value, size_t bufIndex, bool onRising, QString name):
			m_value(value),
			m_bufIdx(bufIndex),
			m_onRising(onRising),
			m_name(name)


		{
		}

	public:
		float m_value;
		size_t m_bufIdx;
		bool m_onRising;
		QString m_name;
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

		bool isBetweenThresholds()
		{
			return m_is_between_trholds;
		}

		virtual inline bool updateState(enum crossEvents crsEvent) = 0;

		static inline enum crossEvents
			get_crossing_type(double samp, double prevSamp,
					double low_trhold, double high_trhold)
		{
			enum crossEvents cross_type = NO_CROSS;

			if (samp > prevSamp) {
				if ((prevSamp <= low_trhold) && (samp >= low_trhold))
					cross_type = POS_CROSS_LOW;
				if ((prevSamp <= high_trhold) && (samp >= high_trhold)) {
					if (cross_type == POS_CROSS_LOW)
						cross_type = POS_CROSS_FULL;
					else
						cross_type = POS_CROSS_HIGH;
				}
			} else if (samp < prevSamp) {
				if ((prevSamp >= low_trhold) && (samp <= low_trhold))
					cross_type = NEG_CROSS_LOW;
				if ((prevSamp >= high_trhold) && (samp <= high_trhold)) {
					if (cross_type == NEG_CROSS_LOW)
						cross_type = NEG_CROSS_FULL;
					else
						cross_type = NEG_CROSS_HIGH;
				}
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
		CrossingDetection(double level, double hysteresis_span,
				const QString &name):
			m_posCrossFound(false),
			m_negCrossFound(false),
			m_crossed(false),
			m_posCrossPoint(0),
			m_negCrossPoint(0),
			m_level(level),
			m_hysteresis_span(hysteresis_span),
			m_low_level(level - hysteresis_span / 2),
			m_high_level(level + hysteresis_span / 2),
			m_name(name),
			m_externList(NULL)
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

		void setExternalList(QList<CrossPoint> *externList)
		{
			m_externList = externList;
		}

		QList<CrossPoint> detectedCrossings()
		{
			return m_detectedCrossings;
		}

		inline void store_closest_val_to_cross_lvl(double *data, size_t i, size_t &point)
		{
			double diff1 = qAbs(data[i - 1] - m_level);
			double diff2 = qAbs(data[i] - m_level);
			double diff;
			size_t idx;

			if (diff1 < diff2) {
				idx = i - 1;
				diff = diff1;
			} else {
				idx = i;
				diff = diff2;
			}

			double old_diff = qAbs(data[point] - m_level);
			if (diff < old_diff)
				point = idx;
		}

		inline void store_first_closest_val_to_cross_lvl(double *data, size_t i, size_t &point)
		{
			double diff1 = qAbs(data[i - 1] - m_level);
			double diff2 = qAbs(data[i] - m_level);

			if (diff1 < diff2)
				point = i - 1;
			else
				point = i;
		}

		inline void crossDetectStep(double *data, size_t i)
		{
			auto cross_type = HystLevelCross::get_crossing_type(data[i],
						data[i - 1], m_low_level, m_high_level);

			if (m_posCross.isBetweenThresholds())
				store_closest_val_to_cross_lvl(data, i, m_posCrossPoint);
			if (m_negCross.isBetweenThresholds())
				store_closest_val_to_cross_lvl(data, i, m_negCrossPoint);

			if (cross_type != HystLevelCross::NO_CROSS) {
				if (!m_posCrossFound) {
					bool old_between_thresh = m_posCross.isBetweenThresholds();
					m_crossed = m_posCross.updateState(cross_type);
					if (!old_between_thresh && m_posCross.isBetweenThresholds())
						store_first_closest_val_to_cross_lvl(data, i, m_posCrossPoint);

					if (m_crossed) {
						m_posCrossFound = true;
						m_negCrossFound = false;
						m_negCross.resetState();
						if (cross_type == HystLevelCross::POS_CROSS_FULL)
							m_posCrossPoint = i;
						m_detectedCrossings.push_back(
							CrossPoint(data[m_posCrossPoint], m_posCrossPoint,
								true, m_name + "R"));
						if (m_externList)
							m_externList->push_back(m_detectedCrossings.last());
					}
				}
				if (!m_negCrossFound) {
					bool old_between_thresh = m_negCross.isBetweenThresholds();
					m_crossed = m_negCross.updateState(cross_type);
					if (!old_between_thresh && m_negCross.isBetweenThresholds())
						store_first_closest_val_to_cross_lvl(data, i, m_negCrossPoint);
					if (m_crossed) {
						m_negCrossFound = true;
						m_posCrossFound = false;
						m_posCross.resetState();
						if (cross_type == HystLevelCross::NEG_CROSS_FULL)
							m_negCrossPoint = i - 1;
						m_detectedCrossings.push_back(
							CrossPoint(data[m_negCrossPoint], m_negCrossPoint,
								false, m_name + "F"));
						if (m_externList)
							m_externList->push_back(m_detectedCrossings.last());
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

		size_t m_posCrossPoint;
		size_t m_negCrossPoint;

		QList<CrossPoint> m_detectedCrossings;
		QList<CrossPoint> *m_externList;

		QString m_name;
	};

	class SpectralDetection {
	public:
	    SpectralDetection(double *data, ssize_t data_length, int harmonics_number):
	    m_data(data),
	    m_data_length(data_length),
	    m_harmonics_number(harmonics_number)
	    {
	    }

	    void calculateSpectralDetectionParameters(double &spur, double &harm_dist, double &noise,
						      double &average_noise, double &signal) {
		struct harmonic_tuple {
		    double harm_value;
		    int harm_bin;
		};

		findHarmonics(m_harmonics_number);

		int spur_bw;
		findSpur(spur, spur_bw, false, m_harmonic_bins[0]);

		if(m_mask.empty())
		    m_mask = calculateAutoMask();
		int noise_bins;
		maskedSumOfSquares(noise_bins, noise, m_mask);

		noise_bins = noise_bins > 1 ? noise_bins : 1;
		noise = average_noise * (m_data_length - 1);

		std::map<int, harmonic_tuple> harmonic;
		for(int i = 0; i < m_harmonics_number; i++)
		{
		    struct harmonic_tuple harm;
		    harm.harm_bin = m_harmonic_bins[i];
		    harm.harm_value = m_harmonics[i];
		    harm.harm_value -= average_noise * m_harmonic_bw[i];
		    harmonic[i+1] = harm;
		}

		spur -= average_noise * spur_bw;
		signal = harmonic[1].harm_value;

		harm_dist = harmonic[2].harm_value + harmonic[3].harm_value +
		    harmonic[4].harm_value + harmonic[5].harm_value;

	    }

	private:

	    std::vector<int> calculateAutoMask() {
		const int BANDWIDTH_DIVIDER = 80;
		const int NUM_INITAL_NOISE_HARMS = 5;

		int bw = int(m_data_length / BANDWIDTH_DIVIDER);
		std::vector<int> mask;
		mask.resize(m_data_length, 1);
		for (int i = 0; i < NUM_INITAL_NOISE_HARMS; i++)
			clearMask(mask, m_harmonic_bins[i] - bw, m_harmonic_bins[i] + bw);
		mask[0] = 0;

		double noise_est;
		int noise_bins;
		maskedSum(noise_bins, noise_est, mask);
		noise_est /= noise_bins;

		 std::fill(mask.begin(), mask.end(), 1);
		 //clear mask la dc.
		 clearMask(mask, 0, 1);
		 int j;
		 int low, high;

		 double sum;

		 for(size_t i = 0; i < m_harmonic_bins.size(); i++)
		 {
		     int h = m_harmonic_bins.at(i);
		     if(mask[h] == 0)
			 continue;
		     j = 1;
		     sum = 0.0F;
		     for(int s = h-j; s < h-j+3; s++)
		     {
			 sum += m_data[s];
		     }
		     sum /= 3;
		     while ( h - j > 0 && mask[h-j] == 1 &&
				    sum > noise_est ) {
			 j++;
			 sum = 0.0F;
			 for(int s = h-j; s < h-j+3; s++)
			 {
			     sum += m_data[s];
			 }
			 sum /= 3;
		     }
		     low = h - j + 1;

		     j = 1;
		     sum = 0.0F;
		     for(int s = h+j-2; s < h+j+1; s++)
		     {
			 sum += m_data[s];
		     }
		     sum /= 3;
		     while ( h + j < m_data_length && mask[h+j] == 1 &&
				     sum > noise_est ) {
			 j++;
			 sum = 0.0F;
			 for(int s = h+j-2; s < h+j+1; s++)
			 {
			     sum += m_data[s];
			 }
			 sum /= 3;
		     }
		     high = h + j - 1;
		     clearMask(mask, low, high);
		 }

		return mask;
	    }

	    void setMask(std::vector<int> &mask, int start, int end, int value)
	    {
		int nyq = mask.size();
		int n = 2 * (nyq - 1);
		std::vector<int> indices;
		for(int i = start; i <= end; i++)
		{
		    int index_value = (i + n) % n;
		    if (index_value > nyq)
			index_value = n - index_value;
		    indices.push_back(index_value);
		}

		for(std::vector<int>::iterator it = indices.begin(); it != indices.end(); ++it)
		{
		    mask.at(*it) = value;
		}
	    }

	    void clearMask(std::vector<int> &mask, int start, int end)
	    {
		setMask(mask, start, end, 0);
	    }

	    void maskedSubset(std::vector<int> &indices, std::vector<int> &new_mask, std::vector<int> mask, int start, int end)
	    {
		int nyq = mask.size();
		int n = 2 * (nyq - 1);
		std::vector<int> mapped_subset;
		for(int i = start; i < end; i++)
		{
		    int index_value = (i + n) % n;
		    if (index_value > nyq)
			index_value = n - index_value;
		    mapped_subset.push_back(index_value);
		}

		for(size_t i = 0; i < mapped_subset.size(); i++)
		{
		    if(mask.at(mapped_subset.at(i)) == 1)
		    {
			indices.push_back(mapped_subset.at(i));
			new_mask.push_back(mask.at(mapped_subset.at(i)));
		    }

		}
	    }

	    void maskedSumOfSquares(int &index, double &value, std::vector<int> mask, int start = 0, int end = 0)
	    {
		 if(end == 0 || end >= m_data_length)
		     end = m_data_length - 1;

		 std::vector<int> indices;
		 std::vector<int> new_mask;
		 maskedSubset(indices, new_mask, mask, start, end);
		 double sum = 0;


		 for(std::vector<int>::iterator it = indices.begin(); it != indices.end(); ++it)
		 {
		    sum = sum + (m_data[*it] * m_data[*it]);
		 }

		 index = indices.size();
		 value = sum;
	    }

	    void maskedSum(int &index, double &value, std::vector<int> mask, int start = 0, int end = 0)
	    {
		 if(end == 0 || end >= m_data_length)
		     end = m_data_length - 1;

		 std::vector<int> indices;
		 std::vector<int> new_mask;
		 maskedSubset(indices, new_mask, mask, start, end);
		 double sum = 0;


		 for(std::vector<int>::iterator it = indices.begin(); it != indices.end(); ++it)
		 {
		    sum = sum + (m_data[*it]);
		 }

		 index = indices.size();
		 value = sum;
	    }

	    void maskedMax(int &index, double &value, std::vector<int> mask, int start = 0, int end = 0)
	    {
		if(end == 0)
		    end = m_data_length - 1;
		double *data_at_index = new double[end]();
		double data_at_index_size;

		std::vector<int> indices;
		std::vector<int> new_mask;
		maskedSubset(indices, new_mask, mask, start, end);

		for(size_t i=0; i<indices.size(); i++)
		{
		    data_at_index[i] = m_data[indices.at(i)];
		}
		data_at_index_size = indices.size();

		getMax(data_at_index, data_at_index_size, index, value);
		if(!indices.empty())
		    index = indices.at(index);
		else
		    index = 0;
		delete[] data_at_index;
	    }

	    void getMax(double *data, int size, int &index, double &value)
	    {
		index = 0;
		value = data[0];
		for (int i = 0; i < size; i++)
		{
		    if (data[i] > value)
			 {
			    index = i;
			    value = data[i];
			 }
		}
	    }

	    void findHarmonics(int max_harmonics)
	    {
		m_harmonic_bins.resize(max_harmonics);
		m_harmonic_bw.resize(max_harmonics);
		m_harmonics.resize(max_harmonics);

		std::fill(m_harmonic_bins.begin(), m_harmonic_bins.end(), 0);
		std::fill(m_harmonic_bw.begin(), m_harmonic_bw.end(), 0);
		std::fill(m_harmonics.begin(), m_harmonics.end(), 0.0);

		//find the fundamental bin (max value) from the data
		int fund_bin = 0;
		double max = 0;
		getMax(m_data, m_data_length, fund_bin, max);

		m_harmonic_bins.at(0) = fund_bin;

		for(int h = 1; h < max_harmonics + 1; h++)
		{
		    std::vector<int> mask;
		    mask.resize(m_data_length, 0);

		    int nominal_bin = h * fund_bin;
		    int h_2 = static_cast<int>(h / 2);
		    if(h > 1)
		    {
			setMask(mask, nominal_bin - h_2, nominal_bin + h_2, 1);
			for(int i = 0; i < h - 1; i++)
			{
			    clearMask(mask, m_harmonic_bins.at(i), m_harmonic_bins.at(i));
			}
			int index;
			double value;
			maskedMax(index, value, mask);
			m_harmonic_bins.at(h-1) = index;
		    }
		    clearMask(mask, nominal_bin - h_2, nominal_bin + h_2);
		    setMask(mask,  m_harmonic_bins.at(h-1) - BW, m_harmonic_bins.at(h-1) + BW, 1);
		    for(int i = 0; i < h - 1; i++)
		    {
			clearMask(mask, m_harmonic_bins.at(i) - BW, m_harmonic_bins.at(i) + BW);
		    }
		    double val_harm;
		    int val_hbws;
		    maskedSumOfSquares(val_hbws, val_harm, mask);
		    m_harmonics[h-1] = val_harm;
		    m_harmonic_bw[h-1] = val_hbws;

		}

	    }

	    void findSpur(double &spur, int& spur_bw, bool find_in_harmonics, int fund_bin) {
		if(find_in_harmonics == true) {
		    int index = std::max_element(m_harmonics.begin() + 1, m_harmonics.end()) - m_harmonics.begin();
		    spur = *std::max_element(m_harmonics.begin() + 1, m_harmonics.end());
		    spur_bw = m_harmonic_bw[index + 1];
		}
		else
		{
		    findSpurInData(spur, spur_bw, fund_bin);
		}
	    }

	    void findSpurInData(double &spur, int& spur_bw, int fund_bin) {
		std::vector<int> mask;
		mask.resize(m_data_length, 1);
		//clear mask la dc.
		clearMask(mask, 0, 1);
		clearMask(mask, fund_bin - BW, fund_bin + BW);
		int index = 0;
		for(size_t i = 0; i < mask.size(); i++)
		{
		    if(mask.at(i) == 1)
		    {
			index = i;
			break;
		    }
		}
		double max_value;
		int max_index;
		maskedSumOfSquares(max_index, max_value, mask, index - BW, index + BW);
		max_index = index;
		int masked_index = 0;

		double value;
		while(index < m_data_length)
		{
		    if(mask.at(index) == 1)
		    {
			maskedSumOfSquares(masked_index, value, mask, index - BW, index + BW);
			if(value > max_value)
			{
			    max_value = value;
			    max_index = index;
			}
		    }
		    index++;
		}

		int spur_bin;
		double spur_value;
		maskedMax(spur_bin, spur_value, mask, max_index - BW, max_index + BW);
		maskedSumOfSquares(spur_bw, spur, mask, spur_bin - BW, spur_bin + BW);
	    }

	private:
	    double *m_data;
	    ssize_t m_data_length;

	    const int BW = 3;
	    std::vector<int> m_harmonic_bins;
	    std::vector<int> m_harmonic_bw;
	    std::vector<double> m_harmonics;
	    int m_harmonics_number;
	    std::vector<int> m_mask;

	};
}

Measure::Measure(int channel, double *buffer, size_t length,
		 const std::function<double(unsigned int, double, bool)> &conversion_fct, bool isTimeDomain):
	m_channel(channel),
	m_buffer(buffer),
	m_buf_length(length),
	m_sample_rate(1.0),
	m_adc_bit_count(0),
	m_cross_level(0),
	m_hysteresis_span(0),
	m_histogram(nullptr),
	m_cross_detect(nullptr),
	m_gatingEnabled(false),
	m_conversion_function(conversion_fct),
	m_isTimeDomain(isTimeDomain),
	m_harmonics_number(5)
{

	if(m_isTimeDomain) {

		// Create a set of measurements
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Period"),
									   MeasurementData::HORIZONTAL, "s",  channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Frequency"),
									   MeasurementData::HORIZONTAL, "Hz", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Min"),
									   MeasurementData::VERTICAL, "V", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Max"),
									   MeasurementData::VERTICAL, "V", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Peak-peak"),
									   MeasurementData::VERTICAL, "V", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Mean"),
									   MeasurementData::VERTICAL, "V", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Cycle Mean"),
									   MeasurementData::VERTICAL, "V", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("RMS"),
									   MeasurementData::VERTICAL, "V", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Cycle RMS"),
									   MeasurementData::VERTICAL, "V", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("AC RMS"),
									   MeasurementData::VERTICAL, "V", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Area"),
									   MeasurementData::VERTICAL, "Vs", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Cycle Area"),
									   MeasurementData::VERTICAL, "Vs", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Low"),
									   MeasurementData::VERTICAL, "V", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("High"),
									   MeasurementData::VERTICAL, "V", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Amplitude"),
									   MeasurementData::VERTICAL, "V", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Middle"),
									   MeasurementData::VERTICAL, "V", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("+Over"),
									   MeasurementData::VERTICAL, "%", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("-Over"),
									   MeasurementData::VERTICAL, "%", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Rise"),
									   MeasurementData::HORIZONTAL, "s", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("Fall"),
									   MeasurementData::HORIZONTAL, "s", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("+Width"),
									   MeasurementData::HORIZONTAL, "s", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("-Width"),
									   MeasurementData::HORIZONTAL, "s", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("+Duty"),
									   MeasurementData::HORIZONTAL, "%", channel));
		m_measurements.push_back(std::make_shared<MeasurementData>(QObject::tr("-Duty"),
									   MeasurementData::HORIZONTAL, "%", channel));
	} else {
	    //Spectral Measurements
	    m_measurements.push_back(std::make_shared<MeasurementData>("Noise_Floor",
		MeasurementData::HORIZONTAL_F, "dB", channel));
	    m_measurements.push_back(std::make_shared<MeasurementData>("SINAD",
		MeasurementData::HORIZONTAL_F, "dB", channel));
	    m_measurements.push_back(std::make_shared<MeasurementData>("SNR",
		MeasurementData::HORIZONTAL_F, "dB", channel));
	    m_measurements.push_back(std::make_shared<MeasurementData>("THD",
		MeasurementData::HORIZONTAL_F, "dB", channel));
	    m_measurements.push_back(std::make_shared<MeasurementData>("THDN",
		MeasurementData::HORIZONTAL_F, "dB", channel));
	    m_measurements.push_back(std::make_shared<MeasurementData>("SFDR",
		MeasurementData::VERTICAL_F, "dBc", channel));
	}

}

void Measure::setConversionFunction(const std::function<double(unsigned int, double, bool)> &fp)
{
	m_conversion_function = fp;

}

bool Measure::highLowFromHistogram(double &low, double &high,
		double min, double max)
{
	bool success = false;
	int *hist = m_histogram;
	int adc_span = 1 << m_adc_bit_count;
	int hlf_scale = adc_span / 2;
	int minRaw = min;
	int maxRaw = max;

	if (m_conversion_function) {
		minRaw = m_conversion_function(m_channel, min, false);
		maxRaw = m_conversion_function(m_channel, max, false);
	}

	minRaw += hlf_scale;
	maxRaw += hlf_scale;

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
		int lowTmp = lowRaw - hlf_scale;
		int highTmp = highRaw - hlf_scale;

		if (m_conversion_function) {
			low = m_conversion_function(m_channel, lowTmp, true);
			high = m_conversion_function(m_channel, highTmp, true);
		}
		success = true;
	}

	return success;
}

void Measure::clearMeasurements()
{
	 for (int i = 0; i < m_measurements.size(); i++)
		m_measurements[i]->setMeasured(false);
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

	if(m_isTimeDomain) {
		measureTimeDomain();
	} else {
		measureSpectral();
	}
}

void Measure::measureTimeDomain()
{
	double period;
	double frequency;
	double rise_time;
	double fall_time;
	double width_p;
	double width_n;
	double duty_p;
	double duty_n;
	double peak_to_peak;
	double min;
	double max;
	double amplitude;
	double high;
	double middle;
	double low;
	double overshoot_p;
	double overshoot_n;
	double mean;
	double cycle_mean;
	double rms;
	double cycle_rms;
	double rms_ac;
	double area;
	double cycle_area;
	double sum;
	double sqr_sum;

	// Cache buffer address, length, ADC bit count
	double *data = m_buffer;
	size_t data_length = m_buf_length;
	size_t count = data_length;
	int adc_span = 1 << m_adc_bit_count;
	int hlf_scale = adc_span / 2;
	bool using_histogram_method = (adc_span > 1);

	int startIndex;
	int endIndex;

	if (qIsNaN(data[0])) {
		return;
	}

	//if gating is enabled measure only on data between the gates
	if(m_gatingEnabled){
		//make sure that start/end indexes are valid
		if(m_startIndex < 0 || m_startIndex > m_buf_length){
			m_startIndex = 0;
		}
		if(m_endIndex < 0 || m_endIndex > m_buf_length ){
			m_endIndex = m_buf_length;
		}

		max = data[m_startIndex];
		min = data[m_startIndex];
		sum = data[m_startIndex];
		sqr_sum = data[m_startIndex] * data[m_startIndex];
		startIndex = m_startIndex+1;
		endIndex = m_endIndex;

		count = (m_endIndex - m_startIndex) ?: 1;
	}
	else{
		max = data[0];
		min = data[0];
		sum = data[0];
		sqr_sum = data[0] * data[0];
		startIndex = 1;
		endIndex = data_length;
	}

	m_cross_detect = new CrossingDetection(m_cross_level, m_hysteresis_span,
			"P");
	if (using_histogram_method)
		m_histogram = new int[adc_span]{};

	for (ssize_t i = startIndex; i < endIndex; i++) {

		if (qIsNaN(data[i])) {
			count--;
			continue;
		}
		// Find level crossings (period detection)
		m_cross_detect->crossDetectStep(data, i);

		// Min
		if (data[i] < min) {
			min = data[i];
		}

		// Max
		if (data[i] > max) {
			max = data[i];
		}

		// Sum of values
		sum += data[i];

		// Sum of the squares of values
		sqr_sum += data[i] * data[i];

		// Build histogram
		if (using_histogram_method) {
			double rawTmp = data[i];
			int raw = 0;
			if (m_conversion_function) {
				raw = (int)m_conversion_function(m_channel, rawTmp, false);
			}
			raw += hlf_scale;

			if (raw >= 0 && raw  < adc_span)
				m_histogram[raw] += 1;
		}
	}

	m_measurements[MIN]->setValue(min);
	m_measurements[MAX]->setValue(max);

	// Peak-to-Peak
	peak_to_peak = qAbs(max - min);
	m_measurements[PEAK_PEAK]->setValue(peak_to_peak);

	// Mean
	mean = sum / count;
	m_measurements[MEAN]->setValue(mean);

	// RMS
	rms = sqrt(sqr_sum / count);
	m_measurements[RMS]->setValue(rms);

	// AC RMS
	rms_ac = sqrt((sqr_sum - 2 * mean * sum +
		count *  mean * mean) / count);
	m_measurements[AC_RMS]->setValue(rms_ac);

	low = min;
	high = max;

	// Try to use Histogram method
	if (using_histogram_method)
		highLowFromHistogram(low, high, min, max);

	// Low, High, Middle, Amplitude, Overshoot positive/negative
	m_measurements[LOW]->setValue(low);
	m_measurements[HIGH]->setValue(high);

	middle = low + (high - low) / 2.0;
	m_measurements[MIDDLE]->setValue(middle);

	amplitude = high - low;
	m_measurements[AMPLITUDE]->setValue(amplitude);

	overshoot_p = (max - high) / amplitude * 100;
	m_measurements[P_OVER]->setValue(overshoot_p);

	overshoot_n = (low - min) / amplitude * 100;
	m_measurements[N_OVER]->setValue(overshoot_n);

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
		period = sample_period * (1 / m_sample_rate);
		m_measurements[PERIOD]->setValue(period);

		frequency = 1 / period;
		m_measurements[FREQUENCY]->setValue(frequency);

		// Find level crossings (10%, 50%, 90%)
		double lowRef = low + (0.1 * amplitude);
		double midRef = low + (0.5 * amplitude);
		double highRef = low + (0.9 * amplitude);

		CrossingDetection cdLow(lowRef, 0.2, "L");
		CrossingDetection cdMid(midRef, 0.2, "M");
		CrossingDetection cdHigh(highRef, 0.2, "H");

		QList<CrossPoint> crossSequence;
		cdLow.setExternalList(&crossSequence);
		cdMid.setExternalList(&crossSequence);
		cdHigh.setExternalList(&crossSequence);

		size_t period_start = periodPoints[0].m_bufIdx;
		size_t period_end = periodPoints[2].m_bufIdx;
		size_t length = period_end - period_start + 1;

		double period_sum = data[period_start];
		double period_sqr_sum = data[period_start] * data[period_start];

		for (size_t i = period_start + 1; i <= period_start + 2 * length; i++) {
			size_t idx = period_start + (i  % length);

			cdLow.crossDetectStep(data, idx);
			cdMid.crossDetectStep(data, idx);
			cdHigh.crossDetectStep(data, idx);
		}

		for (size_t i = period_start + 1; i <= period_end; i++) {
			period_sum += data[i];
			period_sqr_sum += data[i] * data[i];
		}

		for (int i = 1; i < crossSequence.size(); i++) {
			CrossPoint &p0 = crossSequence[i - 1];
			CrossPoint &p1 = crossSequence[i];
			if ((p1.m_bufIdx == p0.m_bufIdx) && (p1.m_onRising == p0.m_onRising)) {
				if ((p0.m_name == "MR" && p1.m_name == "LR") ||
						(p0.m_name == "HR" && p1.m_name == "MR"))
					crossSequence.swap(i, i - 1);
				else if ((p0.m_name == "MF" && p1.m_name == "HF") ||
						(p0.m_name == "LF" && p1.m_name == "MF"))
					crossSequence.swap(i, i - 1);
			}
		}

		QString sequence = "";
		for (int i = 0; i < crossSequence.size(); i++)
			sequence += crossSequence[i].m_name;

		QString periodSequence = "LRMRHRHFMFLF";
		int pos = sequence.indexOf(periodSequence);

		if (pos < 0) {
			qDebug() << "Unable to find 2 transitions for each of the 10%, 50%, 90% levels";
		} else {
			pos /= 2;
			CrossPoint &lowRising = crossSequence[pos];
			CrossPoint &midRising = crossSequence[pos + 1];
			CrossPoint &highRising = crossSequence[pos + 2];
			CrossPoint &highFalling = crossSequence[pos + 3];
			CrossPoint &midFalling = crossSequence[pos + 4];
			CrossPoint &lowFalling = crossSequence[pos + 5];

			//Cycle Mean
			cycle_mean = period_sum / length;
			m_measurements[CYCLE_MEAN]->setValue(cycle_mean);

			//Cycle RMS
			cycle_rms = sqrt(period_sqr_sum / length);
			m_measurements[CYCLE_RMS]->setValue(cycle_rms);

			//Area
			area = sum * (1 / m_sample_rate);
			m_measurements[AREA]->setValue(area);

			//Cycle Area
			cycle_area = period_sum * (1 / m_sample_rate);
			m_measurements[CYCLE_AREA]->setValue(cycle_area);

			// Rise Time
			long long rise = (long long)(highRising.m_bufIdx -
					lowRising.m_bufIdx);
			if (rise < 0)
				rise += length;
			rise_time = rise / m_sample_rate;
			m_measurements[RISE]->setValue(rise_time);

			// Fall Time
			long long fall = (long long)(lowFalling.m_bufIdx -
					highFalling.m_bufIdx);
			if (fall < 0)
				fall += length;
			fall_time = fall / m_sample_rate;
			m_measurements[FALL]->setValue(fall_time);

			// Positive Width
			long long posWidth = (long long)(midFalling.m_bufIdx -
					midRising.m_bufIdx);
			if (posWidth < 0)
				posWidth += length;
			width_p = posWidth / m_sample_rate;
			m_measurements[P_WIDTH]->setValue(width_p);

			// Negative Width
			width_n = period - width_p;
			m_measurements[N_WIDTH]->setValue(width_n);

			// Positive Duty
			duty_p = width_p / period * 100;
			m_measurements[P_DUTY]->setValue(duty_p);

			// Negative Duty
			duty_n = width_n / period * 100;
			m_measurements[N_DUTY]->setValue(duty_n);
		}
	}

	delete m_cross_detect;
	m_cross_detect = NULL;

}

void Measure::measureSpectral() {

    //TODO - reconsider computation of measurements
    double spur, harm_dist, noise, average_noise = 0, signal;

    SpectralDetection detection(m_buffer, m_buf_length, m_harmonics_number);

    detection.calculateSpectralDetectionParameters(spur, harm_dist, noise, average_noise, signal);

    double noise_floor, snr, thd, thdn, sinad, sfdr;
    noise_floor = 10 * log10(average_noise / signal);
    m_measurements[NOISE_FLOOR]->setValue(noise_floor);

    snr = 10 * log10(signal / noise);
    m_measurements[SNR]->setValue(snr);

    thd = harm_dist > 0 ? 10 * log10(harm_dist / signal) : 0;
    m_measurements[THD]->setValue(thd);

    sinad = 10 * log10(signal / (harm_dist + noise));
    m_measurements[SINAD]->setValue(sinad);

    thdn = harm_dist > 0 ? 10 * log10((harm_dist + noise) / signal) : 0;
    m_measurements[THDN]->setValue(thdn);

    sfdr = spur > 0 ? 10 * log10(signal / spur) : 0;
    m_measurements[SFDR]->setValue(sfdr);
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

int Measure::channel() const
{
	return m_channel;
}

void Measure::setChannel(int channel)
{
	if (m_channel != channel) {
		for (int i = 0; i < m_measurements.size(); i++) {
			m_measurements[i]->setChannel(channel);
		}
		m_channel = channel;
	}
}

void Measure::setStartIndex(int index)
{
	m_startIndex = index;
}

void Measure::setEndIndex(int index)
{
	m_endIndex = index;
}

void Measure::setGatingEnabled(bool enable){
	m_gatingEnabled = enable;
}

std::vector<int> Measure::LoadMaskfromFile(std::string file_name)
{
    std::vector<int> mask;
    std::ifstream mask_file(file_name.c_str());
    if(!mask_file.is_open()) throw std::runtime_error("Could not open freq file");

    std::string line = "";
    while(std::getline(mask_file, line)) {

	int val = std::atoi(line.c_str());
	mask.push_back(val);
    }
    return mask;
}

void Measure::setHarmonicNumber(int harmonics_number)
{
    m_harmonics_number = harmonics_number;
}

void Measure::setMask(std::vector<int> mask)
{
    std::vector<int> m_mask = mask;
}

QList<std::shared_ptr<MeasurementData>> Measure::measurments()
{
	return m_measurements;
}

std::shared_ptr<MeasurementData> Measure::measurement(int id)
{
	return m_measurements[id];
}

int Measure::activeMeasurementsCount() const
{
	int count = 0;

	for (int i = 0; i < m_measurements.size(); i++)
		if (m_measurements[i]->enabled())
			count++;

	return count;
}

/*
 * Class MeasurementData implementation
 */

MeasurementData::MeasurementData(const QString& name, axisType axis,
		const QString& unit, int channel):
	m_name(name),
	m_value(0),
	m_measured(false),
	m_enabled(false),
	m_unit(unit),
	m_unitType(DIMENSIONLESS),
	m_channel(channel),
	m_axis(axis)
{
	if (unit.isEmpty())
		m_unitType = DIMENSIONLESS;
	else if (unit == "%")
		m_unitType = PERCENTAGE;
	else if (unit.toLower() == "s" || unit.toLower() == "seconds")
		m_unitType = TIME;
	else if ((unit.toLower() == "db" || unit.toLower() == "decibels"))
		m_unitType = DECIBELS;
	    else if ((unit.toLower() == "dbc" || unit.toLower() == "decibels_to_carrier"))
		m_unitType = DECIBELS_TO_CARRIER;
	else
		m_unitType = METRIC;
}

QString MeasurementData::name() const
{
	return m_name;
}

double MeasurementData::value() const
{
	return m_value;
}

void MeasurementData::setValue(double value)
{
	m_value = value;
	m_measured = true;
}

bool MeasurementData::measured() const
{
	return m_measured;
}

void MeasurementData::setMeasured(bool state)
{
	m_measured = state;
}

bool MeasurementData::enabled() const
{
	return m_enabled;
}

void MeasurementData::setEnabled(bool en)
{
	m_enabled = en;
}

QString MeasurementData::unit() const
{
	return m_unit;
}

MeasurementData::unitTypes MeasurementData::unitType() const
{
	return m_unitType;
}

int MeasurementData::channel() const
{
	return m_channel;
}

void MeasurementData::setChannel(int channel)
{
	m_channel = channel;
}

MeasurementData::axisType MeasurementData::axis() const
{
	return m_axis;
}

/*
 * Class Statistic implementation
 */

Statistic::Statistic():
	m_sum(0),
	m_min(0),
	m_max(0),
	m_dataCount(0),
	m_average(0)
{
}

void Statistic::pushNewData(double data)
{
	m_sum += data;

	if (!m_dataCount) {
		m_min = data;
		m_max = data;
	} else {
		if (data < m_min)
			m_min = data;
		else if (data > m_max)
			m_max = data;
	}

	m_dataCount += 1;
	m_average = m_sum / m_dataCount;
}

void Statistic::clear()
{
	m_sum = 0;
	m_min = 0;
	m_max = 0;
	m_dataCount = 0;
	m_average = 0;
}

double Statistic::average() const
{
	return m_average;
}

double Statistic::min() const
{
	return m_min;
}

double Statistic::max() const
{
	return m_max;
}

double Statistic::numPushedData() const
{
	return m_dataCount;
}
