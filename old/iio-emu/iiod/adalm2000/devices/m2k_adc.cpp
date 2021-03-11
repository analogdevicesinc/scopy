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

#include "m2k_adc.hpp"
#include "utils/attr_ops_xml.hpp"
#include "utils/utility.h"

#define M2K_ADC_CHANNELS 2
#define M2K_ADC_SAMPLE_SIZE 2
#define M2K_ADC_CHANNEL_1 0
#define M2K_ADC_CHANNEL_2 1

using namespace iio_emu;

M2kADC::M2kADC(const char *device_id, QDomDocument *doc)
{
	m_device_id = device_id;
	m_doc = doc;

	m_connections = std::vector<std::pair<AbstractDeviceOut*, unsigned short>>(M2K_ADC_CHANNELS);

	m_range.push_back(new char[IIOD_BUFFER_SIZE]());
	m_range.push_back(new char[IIOD_BUFFER_SIZE]());
	m_hw_offset = std::vector<double>(M2K_ADC_CHANNELS);

	m_filter_compensation_table[1E8] = 1.00;
	m_filter_compensation_table[1E7] = 1.05;
	m_filter_compensation_table[1E6] = 1.10;
	m_filter_compensation_table[1E5] = 1.15;
	m_filter_compensation_table[1E4] = 1.20;
	m_filter_compensation_table[1E3] = 1.26;
}

M2kADC::~M2kADC()
{
	for (auto range : m_range) {
		delete range;
	}
}

int32_t M2kADC::open_dev(size_t sample_size, uint32_t mask, bool cyclic)
{
	return 0;
}

int32_t M2kADC::close_dev()
{
	return 0;
}

int32_t M2kADC::set_buffers_count(uint32_t buffers_count)
{
	return 0;
}

int32_t M2kADC::get_mask(uint32_t *mask)
{
	*mask = 3;
	return 0;
}

ssize_t M2kADC::read_dev(char *pbuf, size_t offset, size_t bytes_count)
{
	loadCalibValues();

	std::vector<int16_t> samples;
	std::vector<double> dac_a_samples = resample(M2K_ADC_CHANNEL_1, bytes_count);
	std::vector<double> dac_b_samples = resample(M2K_ADC_CHANNEL_2, bytes_count);

	for (int i = 0; i < bytes_count / (M2K_ADC_CHANNELS * M2K_ADC_SAMPLE_SIZE); i++) {
		samples.push_back(convertVoltToRaw(dac_a_samples.at(i), M2K_ADC_CHANNEL_1));
		samples.push_back(convertVoltToRaw(dac_b_samples.at(i), M2K_ADC_CHANNEL_2));
	}
	memcpy(pbuf, samples.data(), bytes_count);
	return bytes_count;
}

void M2kADC::connectDevice(unsigned short channel_in, AbstractDeviceOut *deviceOut, unsigned short channel_out)
{
	m_connections.at(channel_in) = std::pair<AbstractDeviceOut*, unsigned short>(deviceOut, channel_out);
}

int16_t M2kADC::convertVoltToRaw(double voltage, unsigned short channel) const
{
	const double correctionGain = 1;
	const double hw_gain = getCalibGain(channel);
	const double filterCompensation = getFilterCompensation();
	const double offset = -m_hw_offset.at(channel);

	return (((voltage - offset) / (correctionGain * filterCompensation) * (2048 * 1.3 * hw_gain) / 0.78));
}

double M2kADC::convertRawToVoltsVerticalOffset(int16_t raw, unsigned short channel) const
{
	const double gain = 1.3;
	const double vref = 1.2;
	const double hw_range_gain = getCalibGain(channel);

	return raw * 2.693 * vref / ((1u << 12u) * hw_range_gain * gain);
}

double M2kADC::getCalibGain(unsigned short channel) const
{
	if (!strncmp(m_range.at(channel), "high", sizeof("high") - 1)) {
		return 0.21229;
	} else if (!strncmp(m_range.at(channel), "low", sizeof("low") - 1)) {
		return 0.02017;
	} else {
		return 0;
	}
}

double M2kADC::getFilterCompensation() const
{
	double compensation = 0.0;
	if(m_filter_compensation_table.find(m_samplerate) != m_filter_compensation_table.end()) {
		compensation = m_filter_compensation_table.at(m_samplerate);
	}

	return compensation;
}

void M2kADC::loadCalibValues()
{
	char tmp_attr[IIOD_BUFFER_SIZE];

	read_channel_attr(m_doc, "iio:device11", "voltage0", false, "gain", m_range.at(M2K_ADC_CHANNEL_1), IIOD_BUFFER_SIZE);
	read_channel_attr(m_doc, "iio:device11", "voltage1", false, "gain", m_range.at(M2K_ADC_CHANNEL_2), IIOD_BUFFER_SIZE);

	read_channel_attr(m_doc, "iio:device2", "voltage2", true, "raw", tmp_attr, IIOD_BUFFER_SIZE);
	m_hw_offset.at(M2K_ADC_CHANNEL_1) = safe_stod(tmp_attr);
	read_channel_attr(m_doc, "iio:device0", "voltage0", false, "calibbias", tmp_attr, IIOD_BUFFER_SIZE);
	m_hw_offset.at(M2K_ADC_CHANNEL_1) -= safe_stod(tmp_attr);
	m_hw_offset.at(M2K_ADC_CHANNEL_1) = convertRawToVoltsVerticalOffset(m_hw_offset.at(M2K_ADC_CHANNEL_1), M2K_ADC_CHANNEL_1);

	read_channel_attr(m_doc, "iio:device2", "voltage3", true, "raw", tmp_attr, IIOD_BUFFER_SIZE);
	m_hw_offset.at(M2K_ADC_CHANNEL_2) = safe_stod(tmp_attr);
	read_channel_attr(m_doc, "iio:device0", "voltage1", false, "calibbias", tmp_attr, IIOD_BUFFER_SIZE);
	m_hw_offset.at(M2K_ADC_CHANNEL_2) -= safe_stod(tmp_attr);
	m_hw_offset.at(M2K_ADC_CHANNEL_2) = convertRawToVoltsVerticalOffset(m_hw_offset.at(M2K_ADC_CHANNEL_2), M2K_ADC_CHANNEL_2);

	read_device_attr(m_doc, "iio:device0", "sampling_frequency", tmp_attr, IIOD_BUFFER_SIZE, IIO_ATTR_TYPE_DEVICE);
	m_samplerate = safe_stod(tmp_attr);

	read_device_attr(m_doc, "iio:device0", "oversampling_ratio", tmp_attr, IIOD_BUFFER_SIZE, IIO_ATTR_TYPE_DEVICE);
	m_oversampling_ratio = std::stoi(tmp_attr);
}

std::vector<double> M2kADC::resample(unsigned short channel, size_t len) const
{
	unsigned int ratio = 1E8 / (m_samplerate/m_oversampling_ratio);

	size_t size = (len / (M2K_ADC_CHANNELS * M2K_ADC_SAMPLE_SIZE)) * ratio;
	std::vector<double> tmp_samples(size);

	m_connections.at(channel).first->transfer_samples_to_RX_device(reinterpret_cast<char *>(tmp_samples.data()), size);

	if (ratio < 2) {
		return tmp_samples;
	}

	std::vector<double> samples;
	for (unsigned int i = 0; i < size; i += ratio) {
		double val = 0;
		for (unsigned int j = 0; j < ratio; j++) {
			val += tmp_samples.at(i+j);
		}

		samples.push_back(val/ratio);
	}
	return samples;
}
