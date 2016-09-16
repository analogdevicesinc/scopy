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

	// Filters applied while decimating affect the amplitude of the received data
	// Use 1.0 value for now, until we do more testing
	m_filt_comp_table[1E8] = 1.0;
	m_filt_comp_table[1E7] = 1.0;
	m_filt_comp_table[1E6] = 1.0;
	m_filt_comp_table[1E5] = 1.0;
	m_filt_comp_table[1E4] = 1.0;
	m_filt_comp_table[1E3] = 1.0;
}

OscADC::~OscADC()
{
}

double OscADC::sampleRate()
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

QList<double>& OscADC::availSamplRates()
{
	return m_availSampRates;
}

unsigned int OscADC::numChannels()
{
	return m_numChannels;
}

struct iio_device* OscADC::iio_adc()
{
	return m_adc;
}

double OscADC::compTable(double samplRate)
{
	return m_filt_comp_table[samplRate];
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
