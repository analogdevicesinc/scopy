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

#include "m2k_logic_rx.hpp"
#include "utils/attr_ops_xml.hpp"
#include "utils/utility.h"

using namespace iio_emu;

static bool getBit(unsigned short number, unsigned int index)
{
	return (number & (1u << index)) >> index;
}

static void setBit(uint16_t &number, unsigned int index)
{
	number |= (1u << index);
}

M2kLogicRX::M2kLogicRX(const char *device_id, QDomDocument *doc)
{
	m_device_id = device_id;
	m_doc = doc;

	m_connections = std::vector<std::pair<AbstractDeviceOut*, unsigned short>>(1);
}

M2kLogicRX::~M2kLogicRX()
{

}

int32_t M2kLogicRX::open_dev(size_t sample_size, uint32_t mask, bool cyclic)
{
	return 0;
}

int32_t M2kLogicRX::close_dev()
{
	return 0;
}

int32_t M2kLogicRX::set_buffers_count(uint32_t buffers_count)
{
	return 0;
}

int32_t M2kLogicRX::get_mask(uint32_t *mask)
{
	return 0;
}

ssize_t M2kLogicRX::read_dev(char *pbuf, size_t offset, size_t bytes_count)
{

	std::vector<uint16_t> samples = resample(0, bytes_count);

	memcpy(pbuf, samples.data(), bytes_count);
	return bytes_count;
}

void M2kLogicRX::connectDevice(unsigned short channel_in, AbstractDeviceOut *deviceOut, unsigned short channel_out)
{
	m_connections.at(channel_in) = std::pair<AbstractDeviceOut*, unsigned short>(deviceOut, channel_out);
}

std::vector<uint16_t> M2kLogicRX::resample(unsigned short channel, size_t len)
{
	loadValues();
	std::vector<uint16_t> samples;
	unsigned int ratio = 1E8 / m_samplerate;

	std::vector<uint16_t> tmp_samples((len/2)*ratio);
	m_connections.at(0).first->transfer_samples_to_RX_device(reinterpret_cast<char *>(tmp_samples.data()), (len/2)*ratio);
	for (unsigned int i = 0; i < (len/2)*ratio; i += ratio) {

		std::vector<unsigned int> bytes(16);
		for (unsigned int k = 0; k < 16; k++) {
			bytes.at(k) = 0;
		}
		for (unsigned int j = 0; j < ratio; j++) {
			for (unsigned int k = 0; k < 16; k++) {
				if (getBit(tmp_samples.at(i+j), k)) {
					bytes.at(k) += 1;
				}
			}
		}
		uint16_t sample = 0;
		for (unsigned int k = 0; k < 16; k++) {
			bytes.at(k) = bytes.at(k) / ratio;
			if (bytes.at(k) >= 0.5) {
				setBit(sample, k);
			}
		}
		samples.push_back(sample);
	}
	return samples;
}

void M2kLogicRX::loadValues()
{
	char tmp_attr[IIOD_BUFFER_SIZE];

	read_device_attr(m_doc, m_device_id, "sampling_frequency", tmp_attr, IIOD_BUFFER_SIZE, IIO_ATTR_TYPE_DEVICE);
	m_samplerate = safe_stod(tmp_attr);
}
