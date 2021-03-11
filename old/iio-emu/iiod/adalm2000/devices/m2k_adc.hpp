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

#ifndef IIO_EMU_M2K_ADC_HPP
#define IIO_EMU_M2K_ADC_HPP

#include "iiod/abstract_device_in.hpp"
#include <QDomDocument>
#include <vector>
#include <map>

namespace iio_emu {

class M2kADC : public AbstractDeviceIn{
public:
	M2kADC(const char *device_id, QDomDocument *doc);
	~M2kADC();

	int32_t open_dev(size_t sample_size, uint32_t mask, bool cyclic) override;
	int32_t close_dev() override;
	int32_t set_buffers_count(uint32_t buffers_count) override;
	int32_t get_mask(uint32_t *mask) override;
	ssize_t read_dev(char *pbuf, size_t offset, size_t bytes_count) override;

	void connectDevice(unsigned short channel_in, AbstractDeviceOut *deviceOut, unsigned short channel_out) override;

private:
	QDomDocument *m_doc;
	std::vector<std::pair<AbstractDeviceOut*, unsigned short>> m_connections;

	double m_samplerate;
	unsigned int m_oversampling_ratio;

	std::vector<char *> m_range;
	std::vector<double> m_hw_offset;
	std::map<double, double> m_filter_compensation_table;

	int16_t convertVoltToRaw(double voltage, unsigned short channel) const;
	double convertRawToVoltsVerticalOffset(int16_t raw, unsigned short channel) const;

	double getCalibGain(unsigned short channel) const;
	double getFilterCompensation() const;
	void loadCalibValues();

	std::vector<double> resample(unsigned short channel, size_t len) const;
};
}

#endif //IIO_EMU_M2K_ADC_HPP
