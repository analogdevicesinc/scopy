/*
 * Copyright (c) 2021 Analog Devices Inc.
 *
 * This file is part of iio-emu
 * (see http://www.github.com/analogdevicesinc/iio-emu).
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

#include "m2k_dac.hpp"
#include "utils/utility.h"
#include "utils/attr_ops_xml.hpp"

using namespace iio_emu;


M2kDAC::M2kDAC(const char *device_id, QDomDocument *doc)
{
	m_device_id = device_id;
	m_doc = doc;
	m_reset_buffer = false;

	m_current_index = 0;

	m_calib_vlsb = 10.0 / ((double)( 1 << 12 ) - 1);

	m_filter_compensation_table[75E6] = 1.00;
	m_filter_compensation_table[75E5] = 1.525879;
	m_filter_compensation_table[75E4] = 1.164153;
	m_filter_compensation_table[75E3] = 1.776357;
	m_filter_compensation_table[75E2] = 1.355253;
	m_filter_compensation_table[75E1] = 1.033976;
}

M2kDAC::~M2kDAC()
{

}

int32_t M2kDAC::open_dev(size_t sample_size, uint32_t mask, bool cyclic)
{
	loadCalibValues();
	m_cyclic = cyclic;
	m_enable = false;
	if (mask) {
		m_enable = true;
	}
	return 0;
}

int32_t M2kDAC::close_dev()
{
	m_samples.clear();
	return 0;
}

int32_t M2kDAC::set_buffers_count(uint32_t buffers_count)
{
	return 0;
}

int32_t M2kDAC::get_mask(uint32_t *mask)
{
	*mask = 0;
	if (m_enable) {
		*mask = 1;
	}
	return 0;
}

ssize_t M2kDAC::write_dev(const char *buf, size_t offset, size_t bytes_count)
{
	if (!m_enable) {
		return 0;
	}

	if (m_reset_buffer) {
		m_samples.clear();
		m_current_index = 0;
		m_reset_buffer = false;
	}

	auto *samples = (int16_t*)buf;
	for (int i = 0; i < bytes_count/2; i++) {
		m_samples.push_back(convertRawToVolts(samples[i]));
	}

	return bytes_count;
}

double M2kDAC::convertRawToVolts(int16_t raw) const
{
	const double filterCompensation = getFilterCompensation();
	return -((((double)(raw >> 4) + 0.5) * filterCompensation * m_calib_vlsb));
}

double M2kDAC::getFilterCompensation() const
{
	return m_filter_compensation_table.at(m_samplerate);
}

void M2kDAC::loadCalibValues()
{
	char tmp_attr[IIOD_BUFFER_SIZE];

	read_device_attr(m_doc, m_device_id, "sampling_frequency", tmp_attr, IIOD_BUFFER_SIZE, IIO_ATTR_TYPE_DEVICE);
	m_samplerate = safe_stod(tmp_attr);

	read_device_attr(m_doc, m_device_id, "oversampling_ratio", tmp_attr, IIOD_BUFFER_SIZE, IIO_ATTR_TYPE_DEVICE);
	m_oversampling_ratio = std::stoi(tmp_attr);
}

std::vector<double> M2kDAC::resample()
{
	loadCalibValues();
	unsigned int ratio = 75E6 / (m_samplerate/m_oversampling_ratio );

	if (ratio < 2) {
		return m_samples;
	}

	std::vector<double> samples;
	for (unsigned int i = 0; i < m_samples.size() - 1; i++) {
		double val = (m_samples.at(i+1) - m_samples.at(i)) / (ratio + 1);
		samples.push_back(m_samples.at(i));
		for (unsigned int j = 0; j < ratio; j++) {
			samples.push_back(samples.back() + val);
		}
	}
	samples.push_back(m_samples.back());

	return samples;
}

ssize_t M2kDAC::transfer_mem_to_dev(size_t bytes_count)
{
	m_samples = resample();
	m_reset_buffer = true;
	return 0;
}

void M2kDAC::transfer_samples_to_RX_device(char *buf, size_t samples_count)
{
	if (m_samples.empty()) {
		m_samples = std::vector<double>(samples_count);
	}

	std::vector<double> samples;

	for (int i = 0; i < samples_count; i++) {
		if (m_cyclic) {
			samples.push_back(m_samples.at(m_current_index));
			m_current_index++;
			if (m_current_index >= m_samples.size()) {
				m_current_index = 0;
			}
		} else {
			if (!m_samples.empty()) {
				samples.push_back(m_samples.front());
				m_samples.erase(m_samples.begin());
			} else {
				samples.at(i) = 0;
			}
		}
	}
	memcpy(buf, samples.data(), samples_count * sizeof(double));
}
