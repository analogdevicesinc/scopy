#include "osc_adc.h"
#include "filter.hpp"
#include <iio.h>
#include <QString>

using namespace adiscope;

OscADC::OscADC(struct iio_context *ctx, const Filter *filt):
	m_ctx(ctx),
	m_adc(filt->find_device(ctx, TOOL_OSCILLOSCOPE)),
	m_sampleRate(0),
	m_numChannels(0)
{
	double samplRate;

	QStringList list = get_avail_options_list(m_adc,
					"sampling_frequency_available");
	for (int i = 0; i < list.size(); i++)
		m_availSampRates.append(list.at(i).toDouble());

	iio_device_attr_read_double(m_adc, "sampling_frequency", &samplRate);
	setSampleRate(samplRate);

	m_numChannels = get_nb_channels(m_adc);

	// Channel default gains
	for (int i = 0; i < m_numChannels; i++) {
		m_channel_gain_list.push_back(1.0);
		m_channel_hw_gain_list.push_back(0.02);
		m_channel_hw_offset_list.push_back(0.0);
	}

	// Filters applied while decimating affect the amplitude of the received data
	m_filt_comp_table[1E8] = 1.00;
	m_filt_comp_table[1E7] = 1.05;
	m_filt_comp_table[1E6] = 1.10;
	m_filt_comp_table[1E5] = 1.15;
	m_filt_comp_table[1E4] = 1.20;
	m_filt_comp_table[1E3] = 1.26;
}

OscADC::~OscADC()
{
}

double OscADC::sampleRate() const
{
	return m_sampleRate;
}

void OscADC::setSampleRate(double newSr)
{
	if (m_sampleRate == newSr)
		return;

	m_sampleRate = newSr;

	iio_device_attr_write_longlong(m_adc, "sampling_frequency", newSr);
}

QList<double> OscADC::availSamplRates() const
{
	return m_availSampRates;
}

unsigned int OscADC::numChannels() const
{
	return m_numChannels;
}

double OscADC::channelGain(unsigned int chnIdx) const
{
	double gain = 0;

	if (chnIdx < m_numChannels)
		gain = m_channel_gain_list[chnIdx];

	return gain;
}

void OscADC::setChannelGain(unsigned int chnIdx, double gain)
{
	if (chnIdx < m_numChannels)
		m_channel_gain_list[chnIdx] = gain;
}

double OscADC::channelHwGain(unsigned int chnIdx) const
{
	double gain = 0;

	if (chnIdx < m_numChannels)
		gain = m_channel_hw_gain_list[chnIdx];

	return gain;
}

void OscADC::setChannelHwGain(unsigned int chnIdx, double gain)
{
	if (chnIdx < m_numChannels)
		m_channel_hw_gain_list[chnIdx] = gain;
}

double OscADC::channelHwOffset(unsigned int chnIdx) const
{
	double offset = 0;

	if (chnIdx < m_numChannels)
		offset = m_channel_hw_offset_list[chnIdx];

	return offset;
}

void OscADC::setChannelHwOffset(unsigned int chnIdx, double offset)
{
	if (chnIdx < m_numChannels)
		m_channel_hw_offset_list[chnIdx] = offset;
}

struct iio_device* OscADC::iio_adc() const
{
	return m_adc;
}

double OscADC::compTable(double samplRate) const
{
	return m_filt_comp_table.at(samplRate);
}

unsigned int OscADC::get_nb_channels(struct iio_device *dev)
{
	unsigned int nb = 0;

	for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if (!iio_channel_is_output(chn) &&
				iio_channel_is_scan_element(chn))
			nb++;
	}

	return nb;
}

QStringList OscADC::get_avail_options_list(struct iio_device *dev,
					const char *attr_name)
{
	char buffer[8192];
	ssize_t ret;
	QStringList list;

	ret = iio_device_attr_read(dev, attr_name, buffer, sizeof(buffer));
	if (ret >= 0) {
		QString s(buffer);
		list = s.split(" ", QString::SkipEmptyParts);
	}

	return list;
}
