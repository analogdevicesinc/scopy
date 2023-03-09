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
#include "hw_dac.h"
#include "osc_adc.h" // because it contains the IioUtils class (TO DO: Move IioUils in separate file)

#include <iio.h>

using namespace adiscope;

/*
 * class GenericDac
 */

GenericDac::GenericDac(struct iio_context *ctx, struct iio_device *dac_dev):
	m_ctx(ctx),
	m_dac(dac_dev),
	m_dac_bits(0)
{
	m_dac_channels = IioUtils::pick_channels_with_direction(
			IioUtils::scan_elem_channel_list(dac_dev), true);

	if (m_dac_channels.size() > 0) {
		m_dac_bits = iio_channel_get_data_format(
			m_dac_channels[0])->bits;
	}
	setVOutH(5.0);
	setVOutL(-5.0);
	setMaxNumberOfSamples(4 * 1024 * 1024 / 2); // 2 Megasamples
	m_sample_rate = 1e6;

}

GenericDac::~GenericDac()
{
}

struct iio_context * GenericDac::iio_context() const
{
	return m_ctx;
}

struct iio_device * GenericDac::iio_dac_dev() const
{
	return m_dac;
}

uint GenericDac::numDacChannels() const
{
	return m_dac_channels.size();
}

QList<struct iio_channel *> GenericDac::dacChannelList() const
{
	return m_dac_channels;
}

uint GenericDac::numDacBits() const
{
	return m_dac_bits;
}

double GenericDac::sampleRate() const
{
	return m_sample_rate;
}

double GenericDac::readSampleRate()
{
	iio_device_attr_read_double(m_dac, "sampling_frequency",
		&m_sample_rate);

	return m_sample_rate;
}

size_t GenericDac::maxNumberOfSamples()
{
	return m_maxNumberOfSamples;
}
void GenericDac::setMaxNumberOfSamples(size_t val)
{
	m_maxNumberOfSamples = val;
}

void GenericDac::setSampleRate(double sr)
{
	iio_device_attr_write_double(m_dac, "sampling_frequency", sr);
	m_sample_rate = sr;
}

double GenericDac::vlsb() const
{
	// TO DO: implement a generic formula
	return 0;
}

void GenericDac::setVlsb(double vlsb)
{
	// TO DO: implement a generic formula
}

double GenericDac::convVoltsToSample(double volts)
{
        return 0;
}

double GenericDac::vOutL() const
{
	return m_vOutL;
}

void GenericDac::setVOutL(double value)
{
	m_vOutL = value;
}

double GenericDac::vOutH() const
{
	return m_vOutH;
}

void GenericDac::setVOutH(double value)
{
	m_vOutH = value;
}

/*
 * Class M2kDac
 */

M2kDac::M2kDac(struct iio_context *ctx, struct iio_device *dac_dev):
	GenericDac(ctx, dac_dev)
{
	// Filters applied while interpolating affect the amplitude of the
	// transmitted data

	setVOutH(5.0);
	setVOutL(-5.0);
	setMaxNumberOfSamples(4 * 1024 * 1024 / 2); // 2 Megasamples
	m_vlsb = 10.0 / ((double)( 1 << 12 ));
	m_filt_comp_table[75E6] = 1.00;
	m_filt_comp_table[75E5] = 1.525879;
	m_filt_comp_table[75E4] = 1.164153;
	m_filt_comp_table[75E3] = 1.776357;
	m_filt_comp_table[75E2] = 1.355253;
	m_filt_comp_table[75E1] = 1.033976;
}

M2kDac::~M2kDac()
{
}

double M2kDac::compTable(double samplRate) const
{
	return m_filt_comp_table.at(samplRate);
}

double M2kDac::vlsb() const
{
	return m_vlsb;
}

void M2kDac::setVlsb(double vlsb)
{
	m_vlsb = vlsb;
}

double M2kDac::convVoltsToSample(double volts)
{
	return (-1 * (1 / vlsb()) * 16) / compTable(sampleRate());
}
