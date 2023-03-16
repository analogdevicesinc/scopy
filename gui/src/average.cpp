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

#include "average.h"
#include <algorithm>
#include <cstring>
#include <math.h>
#include <memory>

using namespace adiscope;

/*
 * class SpectrumAverage
 */
SpectrumAverage::SpectrumAverage(unsigned int data_width, unsigned int history,
				 bool history_en):
	m_data_width(data_width), m_history_size(history),
	m_history_enabled(history_en)
{
	if (data_width < 1)
		m_data_width = 1;

	if (history < 1)
		m_history_size = 1;

	m_average = new double[m_data_width];
}

SpectrumAverage::~SpectrumAverage()
{
	delete[] m_average;
}

void SpectrumAverage::getAverage(double *out_data,
	unsigned int num_samples) const
{
	unsigned int size = std::min(m_data_width, num_samples);

	std::memcpy(out_data, m_average, size * sizeof(double));
}

unsigned int SpectrumAverage::dataWidth() const
{
	return m_data_width;
}

unsigned int SpectrumAverage::history() const
{
	return m_history_size;
}

void SpectrumAverage::setHistory(unsigned int history)
{
	m_history_size = history;
}

bool SpectrumAverage::historyEnabled() const
{
	return m_history_enabled;
}


/*
 * class AverageHistoryOne
 */
AverageHistoryOne::AverageHistoryOne(unsigned int data_width, unsigned history):
	SpectrumAverage(data_width, history, false), m_anyDataPushed(false)
{
}

void AverageHistoryOne::reset()
{
	m_anyDataPushed = false;
}

/*
 * class AverageHistoryN
 */
AverageHistoryN::AverageHistoryN(unsigned int data_width, unsigned int history):
	SpectrumAverage(data_width, history, true), m_insert_index(0),
	m_inserted_count(0)
{
	alloc_history(m_data_width, m_history_size);
}

AverageHistoryN::~AverageHistoryN()
{
	free_history();
}

void AverageHistoryN::reset()
{
	m_inserted_count = 0;
	m_insert_index = 0;
}

void AverageHistoryN::alloc_history(unsigned int data_width,
	unsigned int history_size)
{
	std::unique_lock<std::mutex> lock(m_history_mutex);
	m_history = new double*[history_size];
	for (unsigned int i = 0; i < history_size; i++)
		m_history[i] = new double[data_width];

}

void AverageHistoryN::free_history()
{
	std::unique_lock<std::mutex> lock(m_history_mutex);
	for (unsigned int i = 0; i < m_history_size; i++)
		delete[] m_history[i];
	delete[] m_history;
}

void AverageHistoryN::setHistory(unsigned int history)
{
	double **tmp_history;
	tmp_history = new double*[history];

	for (unsigned int i = 0; i < history; i++) {
		tmp_history[i] = new double[m_data_width];
	}

	if (history > m_history_size) {
		for (unsigned int i = 0; i < m_history_size; i++) {
			std::memcpy(tmp_history[i], m_history[i],
				    m_data_width * sizeof(double));
		}
	} else {
		int start_index = (m_insert_index > history) ? (m_insert_index - history) : 0;
		int remaining_samples = std::min(m_inserted_count, history);
		int tmp_i = 0;

		for (unsigned int i = start_index; i < m_insert_index; i++) {
			std::memcpy(tmp_history[tmp_i], m_history[i],
				    m_data_width * sizeof(double));
			remaining_samples--;
			tmp_i++;
		}

		for (unsigned int i = m_history_size - remaining_samples; i < m_history_size; i++) {
			std::memcpy(tmp_history[tmp_i], m_history[i],
				    m_data_width * sizeof(double));
			tmp_i++;
		}

		if (m_insert_index > history) {
			m_insert_index = 0;
		}
		m_inserted_count = std::min(m_inserted_count, history);
	}

	free_history();

	std::unique_lock<std::mutex> lock(m_history_mutex);
	m_history = tmp_history;
	SpectrumAverage::setHistory(history);
}

void AverageHistoryN::pushNewData(double *data)
{
	std::unique_lock<std::mutex> lock(m_history_mutex);
	std::memcpy(m_history[m_insert_index], data,
		m_data_width * sizeof(double));
	m_insert_index = (m_insert_index + 1) % m_history_size;
	m_inserted_count = std::min(m_inserted_count + 1, m_history_size);
}

/*
 * class PeakHoldContinuous
 */
PeakHoldContinuous::PeakHoldContinuous(unsigned int data_width,
	unsigned int history): AverageHistoryOne(data_width, history)
{
}

void PeakHoldContinuous::pushNewData(double *data)
{
	if (m_anyDataPushed) {
		for (unsigned int i = 0; i < m_data_width; i++)
			m_average[i] = std::max(data[i], m_average[i]);
	} else {
		std::memcpy(m_average, data, m_data_width * sizeof(double));
		m_anyDataPushed = true;
	}
}

/*
 * class MinHoldContinuous
 */
MinHoldContinuous::MinHoldContinuous(unsigned int data_width,
	unsigned int history): AverageHistoryOne(data_width, history)
{
}

void MinHoldContinuous::pushNewData(double *data)
{
	if (m_anyDataPushed) {
		for (unsigned int i = 0; i < m_data_width; i++)
			m_average[i] = std::min(data[i], m_average[i]);
	} else {
		std::memcpy(m_average, data, m_data_width * sizeof(double));
		m_anyDataPushed = true;
	}
}

/*
 * class ExponentialRMS
 */
ExponentialRMS::ExponentialRMS(unsigned int data_width, unsigned int history):
	AverageHistoryOne(data_width, history)
{
}

void ExponentialRMS::pushNewData(double *data)
{
	if (m_anyDataPushed) {
		for (unsigned int i = 0; i < m_data_width; i++)
			m_average[i] = (data[i] * data[i] + (m_history_size - 1)
				* m_average[i]) / m_history_size;
	} else {
		for (unsigned int i = 0; i < m_data_width; i++)
			m_average[i] = data[i] * data[i];
		m_anyDataPushed = true;
	}
}

/*
 * class ExponentialAverage
 */
ExponentialAverage::ExponentialAverage(unsigned int data_width, unsigned int history):
	AverageHistoryOne(data_width, history)
{
}

void ExponentialAverage::pushNewData(double *data)
{
	if (m_anyDataPushed) {
		for (unsigned int i = 0; i < m_data_width; i++)
			m_average[i] = (data[i] + (m_history_size - 1)
				* m_average[i]) / m_history_size;
	} else {
		std::memcpy(m_average, data, m_data_width * sizeof(double));
		m_anyDataPushed = true;
	}
}

/*
 * class PeakHold
 */
PeakHold::PeakHold(unsigned int data_width, unsigned int history):
	AverageHistoryN(data_width, history)
{
}

void PeakHold::pushNewData(double *data)
{
	double *peaks = m_average;

	if (m_inserted_count == 0 || m_history_size == 1) {
		std::memcpy(peaks, data, m_data_width * sizeof(double));
	} else {
		for (unsigned int i = 0; i < m_data_width; i++) {
			if (data[i] > peaks[i])
				peaks[i] = data[i];

			if (m_inserted_count != m_history_size)
				continue;

			// If the value that we're about to drop (overwrite) is
			// currently the peak we need to find a new peak
			if (m_history[m_insert_index][i] == peaks[i])
				peaks[i] = getPeakFromHistoryColumn(i);
		}
	}

	// Let the base class handle the data storing
	AverageHistoryN::pushNewData(data);
}

double PeakHold::getPeakFromHistoryColumn(unsigned int col)
{
	if (m_inserted_count < 2)
		return m_history[0][col];

	unsigned int start = (m_insert_index != 0) ? 0 : 1;
	double peak = m_history[start][col];

	for (unsigned int i = start + 1; i < m_inserted_count; i++) {
		if (i != m_insert_index && m_history[i][col] > peak)
			peak = m_history[i][col];
	}

	return peak;
}

/*
 * class MinHold
 */
MinHold::MinHold(unsigned int data_width, unsigned int history):
	AverageHistoryN(data_width, history)
{
}

void MinHold::pushNewData(double *data)
{
	double *mins = m_average;

	if (m_inserted_count == 0 || m_history_size == 1) {
		std::memcpy(mins, data, m_data_width * sizeof(double));
	} else {
		for (unsigned int i = 0; i < m_data_width; i++) {
			if (data[i] < mins[i])
				mins[i] = data[i];

			if (m_inserted_count != m_history_size)
				continue;

			// If the value that we're about to drop (overwrite) is
			// currently the min we need to find a new min
			if (m_history[m_insert_index][i] == mins[i])
				mins[i] = getMinFromHistoryColumn(i);
		}
	}

	// Let the base class handle the data storing
	AverageHistoryN::pushNewData(data);
}

double MinHold::getMinFromHistoryColumn(unsigned int col)
{
	if (m_inserted_count < 2)
		return m_history[0][col];

	unsigned int start = (m_insert_index != 0) ? 0 : 1;
	double min = m_history[start][col];

	for (unsigned int i = start + 1; i < m_inserted_count; i++) {
		if (i != m_insert_index && m_history[i][col] < min)
			min = m_history[i][col];
	}

	return min;
}

/*
 * class LinearRMSOne
 */
LinearRMSOne::LinearRMSOne(unsigned int data_width, unsigned int history):
	AverageHistoryOne(data_width, history)
{
	m_sqr_sums = new double[m_data_width];
	m_inserted_count = 0;
}

LinearRMSOne::~LinearRMSOne()
{
	delete[] m_sqr_sums;
}

void LinearRMSOne::pushNewData(double *data)
{
	if (m_anyDataPushed) {
		if (m_inserted_count < m_history_size) {
			for (unsigned int i = 0; i < m_data_width; i++) {
				m_sqr_sums[i] += (data[i] * data[i]);
			}
			m_inserted_count++;
		} else if (m_inserted_count != 0) {
			for (unsigned int i = 0; i < m_data_width; i++) {
				m_average[i] = m_sqr_sums[i] / m_inserted_count;
				m_sqr_sums[i] = 0;
			}
			m_inserted_count = 0;
		}
	} else {
		std::memcpy(m_average, data, m_data_width * sizeof(double));
		m_anyDataPushed = true;
	}

}


/*
 * class LinearAverageOne
 */
LinearAverageOne::LinearAverageOne(unsigned int data_width, unsigned int history):
	AverageHistoryOne(data_width, history)
{
	m_sums = new double[m_data_width];
	m_inserted_count = 0;
}

LinearAverageOne::~LinearAverageOne()
{
	delete[] m_sums;
}

void LinearAverageOne::pushNewData(double *data)
{
	if (m_anyDataPushed) {
		if (m_inserted_count < m_history_size) {
			for (unsigned int i = 0; i < m_data_width; i++) {
				m_sums[i] += data[i];
			}
			m_inserted_count++;
		} else if (m_inserted_count != 0) {
			for (unsigned int i = 0; i < m_data_width; i++) {
				m_average[i] = m_sums[i] / m_inserted_count;
				m_sums[i] = 0;
			}
			m_inserted_count = 0;
		}
	} else {
		std::memcpy(m_average, data, m_data_width * sizeof(double));
		m_anyDataPushed = true;
	}
}


/*
 * class LinearRMS
 */
LinearRMS::LinearRMS(unsigned int data_width, unsigned int history):
	AverageHistoryN(data_width, history)
{
	m_sqr_sums = new double[data_width]();
}

LinearRMS::~LinearRMS()
{
	delete[] m_sqr_sums;
}

void LinearRMS::pushNewData(double *data)
{
	if (m_inserted_count != m_history_size) {
		for (unsigned int i = 0; i < m_data_width; i++) {
			m_sqr_sums[i] += (data[i] * data[i]);
		}
	} else {
		for (unsigned int i = 0; i < m_data_width; i++) {
			m_sqr_sums[i] -= (m_history[m_insert_index][i] *
				m_history[m_insert_index][i]);
			m_sqr_sums[i] += (data[i] * data[i]);
		}
	}

	// Let the base class handle the data storing
	AverageHistoryN::pushNewData(data);
}

void LinearRMS::getAverage(double *out_data, unsigned int num_samples) const
{
	unsigned int num = std::min(m_data_width, num_samples);

	for (unsigned int i = 0; i < num; i++)
		out_data[i] = sqrt(m_sqr_sums[i] / m_inserted_count);
}

void LinearRMS::reset()
{
	std::fill_n(m_sqr_sums, m_data_width, 0);
	AverageHistoryN::reset();
}

/*
 * class LinearAverage
 */
LinearAverage::LinearAverage(unsigned int data_width, unsigned int history):
	AverageHistoryN(data_width, history)
{
	m_sums = new double[data_width]();
}

LinearAverage::~LinearAverage()
{
	delete[] m_sums;
}

void LinearAverage::pushNewData(double *data)
{
	if (m_inserted_count != m_history_size) {
		for (unsigned int i = 0; i < m_data_width; i++) {
			m_sums[i] += data[i];
		}
	} else {
		for (unsigned int i = 0; i < m_data_width; i++) {
			m_sums[i] -= m_history[m_insert_index][i];
			m_sums[i] += data[i];
		}
	}

	// Let the base class handle the data storing
	AverageHistoryN::pushNewData(data);
}

void LinearAverage::getAverage(double *out_data, unsigned int num_samples) const
{
	unsigned int num = std::min(m_data_width, num_samples);

	for (unsigned int i = 0; i < num; i++)
		out_data[i] = m_sums[i] / m_inserted_count;
}

void LinearAverage::reset()
{
	std::fill_n(m_sums, m_data_width, 0);
	AverageHistoryN::reset();
}
