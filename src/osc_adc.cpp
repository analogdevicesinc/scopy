#include "osc_adc.h"
#include "filter.hpp"
#include <iio.h>
#include <QString>

using namespace adiscope;

/*
 * class IioUtils
 */

QStringList IioUtils::available_options_list(struct iio_device *dev,
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

QList<struct iio_channel *> IioUtils::scan_elem_channel_list(
		struct iio_device *dev)
{
	QList<struct iio_channel *> list;

	for (unsigned int i = 0; i < iio_device_get_channels_count(dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);

		if (!iio_channel_is_scan_element(chn))
			continue;

		list.push_back(chn);
	}

	return list;
}

QList<struct iio_channel *> IioUtils::pick_channels_with_direction(
		const QList<struct iio_channel *>& list, bool output)
{
	QList<struct iio_channel *> return_list;

	for (int i = 0; i < list.size(); i++) {
		struct iio_channel *chn = list.at(i);

		if (output && iio_channel_is_output(chn))
			return_list.push_back(chn);
		else if (!output && !iio_channel_is_output(chn))
			return_list.push_back(chn);
	}

	return return_list;
}

/*
 * class GenericAdc
 */

GenericAdc::GenericAdc(struct iio_context *ctx, struct iio_device *adc_dev):
	m_ctx(ctx),
	m_adc(adc_dev),
	m_adc_bits(0)
{
	m_adc_channels = IioUtils::pick_channels_with_direction(
			IioUtils::scan_elem_channel_list(adc_dev), false);

	if (m_adc_channels.size() > 0) {
		m_adc_bits = iio_channel_get_data_format(
			m_adc_channels[0])->bits;
	}

	m_sample_rate = readSampleRate();
}

GenericAdc::~GenericAdc()
{
}

struct iio_context * GenericAdc::iio_context() const
{
	return m_ctx;
}

struct iio_device * GenericAdc::iio_adc_dev() const
{
	return m_adc;
}

QList<struct iio_channel *> GenericAdc::adcChannelList() const
{
	return m_adc_channels;
}

uint GenericAdc::numAdcChannels() const
{
	return m_adc_channels.size();
}

uint GenericAdc::numAdcBits() const
{
	return m_adc_bits;
}

double GenericAdc::sampleRate() const
{
	return m_sample_rate;
}

double GenericAdc::readSampleRate()
{
	iio_device_attr_read_double(m_adc, "sampling_frequency",
		&m_sample_rate);

	return m_sample_rate;
}

void GenericAdc::setSampleRate(double sr)
{
	iio_device_attr_write_double(m_adc, "sampling_frequency", sr);
	m_sample_rate = sr;
}

double GenericAdc::convSampleToVolts(uint chnIdx, double sample) const
{
	return sample;
}

double GenericAdc::convVoltsToSample(uint chnIdx, double volts) const
{
	return volts;
}

double GenericAdc::convSampleDiffToVoltsDiff(uint chnIdx, double smpl)const
{
	return smpl;
}

double GenericAdc::convVoltsDiffToSampleDiff(uint chnIdx, double v) const
{
	return v;
}

GenericAdc::settings_uptr GenericAdc::getCurrentHwSettings()
{
	settings_uptr settings_uptr(new Settings);

	settings_uptr->sample_rate = readSampleRate();

	return settings_uptr;
}

void GenericAdc::setHwSettings(GenericAdc::Settings *settings)
{
	setSampleRate(settings->sample_rate);
}

/*
 * Class M2kAdc
 */

M2kAdc::M2kAdc(struct iio_context *ctx, struct iio_device *adc_dev):
	GenericAdc(ctx, adc_dev)
{
	apply_m2k_fixes();

	// Hardware gain channels
	struct iio_device *m2k_fabric = iio_context_find_device(ctx,
		"m2k-fabric");
	m_gain_channels.push_back(iio_device_find_channel(m2k_fabric,
		"voltage0", false));
	m_gain_channels.push_back(iio_device_find_channel(m2k_fabric,
		"voltage1", false));

	// Hardware offset cannels
	struct iio_device *ad5625 = iio_context_find_device(ctx, "ad5625");
	m_offset_channels.push_back(iio_device_find_channel(ad5625, "voltage2",
		true));
	m_offset_channels.push_back(iio_device_find_channel(ad5625, "voltage3",
		true));

	// Available frequencies list
	QStringList list = IioUtils::available_options_list(adc_dev,
					"sampling_frequency_available");
	for (int i = 0; i < list.size(); i++)
		m_availSampRates.append(list.at(i).toDouble());


	// Filters applied while decimating affect the amplitude of the received
	// data
	m_filt_comp_table[1E8] = 1.00;
	m_filt_comp_table[1E7] = 1.05;
	m_filt_comp_table[1E6] = 1.10;
	m_filt_comp_table[1E5] = 1.15;
	m_filt_comp_table[1E4] = 1.20;
	m_filt_comp_table[1E3] = 1.26;

	// Channel defaults
	for (uint i = 0; i < numAdcChannels(); i++) {
		m_chn_corr_offsets.push_back(0.0);
		m_chn_corr_gains.push_back(1.0);
		m_chn_hw_offsets.push_back(0.0);
		m_chn_hw_gain_modes.push_back(GainMode::LOW_GAIN_MODE);
	}
}

M2kAdc::~M2kAdc()
{
}

void M2kAdc::apply_m2k_fixes()
{
	struct iio_device *dev = iio_context_find_device(iio_context(),
		"ad9963");

	/* Configure TX path */
	iio_device_reg_write(dev, 0x68, 0x1B);  // IGAIN1 +-6db  0.25db steps
	iio_device_reg_write(dev, 0x6B, 0x1B);  //
	iio_device_reg_write(dev, 0x69, 0x1C);  // IGAIN2 +-2.5%
	iio_device_reg_write(dev, 0x6C, 0x1C);
	iio_device_reg_write(dev, 0x6A, 0x20);  // IRSET +-20%
	iio_device_reg_write(dev, 0x6D, 0x20);
}

double M2kAdc::chnCorrectionOffset(uint chnIdx) const
{
	return m_chn_corr_offsets[chnIdx];
}

void M2kAdc::setChnCorrectionOffset(uint chnIdx, double offset)
{
	m_chn_corr_offsets[chnIdx] = offset;
}

double M2kAdc::chnCorrectionGain(uint chnIdx) const
{
	return m_chn_corr_gains[chnIdx];
}

void M2kAdc::setChnCorrectionGain(uint chnIdx, double gain)
{
	m_chn_corr_gains[chnIdx] = gain;
}

double M2kAdc::chnHwOffset(uint chnIdx) const
{
	return m_chn_hw_offsets[chnIdx];
}

void M2kAdc::setChnHwOffset(uint chnIdx, double offset)
{
	double gain = 1.3;
	double vref = 1.2;
	double hw_chn_gain = gainAt(chnHwGainMode(chnIdx));
	int raw_offset = (int)(offset * (1 << numAdcBits()) * hw_chn_gain *
		gain / 2.693 / vref) + m_chn_corr_offsets[chnIdx];

	iio_channel_attr_write_longlong(m_offset_channels[chnIdx], "raw",
		(long long)raw_offset);

	m_chn_hw_offsets[chnIdx] = offset;
}

M2kAdc::GainMode M2kAdc::chnHwGainMode(uint chnIdx) const
{
	return m_chn_hw_gain_modes[chnIdx];
}

void M2kAdc::setChnHwGainMode(uint chnIdx, GainMode gain_mode)
{
	const char *str_gain_mode = (gain_mode == GainMode::HIGH_GAIN_MODE) ?
		"high" : "low";

	iio_channel_attr_write_raw(m_gain_channels[chnIdx], "gain",
		str_gain_mode, strlen(str_gain_mode));

	m_chn_hw_gain_modes[chnIdx] = gain_mode;
}

double M2kAdc::gainAt(GainMode gain_mode) const
{
	if (gain_mode == LOW_GAIN_MODE)
		return 0.02;
	else if (gain_mode == HIGH_GAIN_MODE)
		return 0.212;
	else
		return 0;
}

QPair<double, double> M2kAdc::inputRange(GainMode gain_mode) const
{
	if (gain_mode == LOW_GAIN_MODE)
		return QPair<double, double>(-25.0, 25.0);
	else if (gain_mode == HIGH_GAIN_MODE)
		return QPair<double, double>(-2.5, 2.5);
	else
		return QPair<double, double>(0, 0);
}

QList<double> M2kAdc::availSamplRates() const
{
	return m_availSampRates;
}

double M2kAdc::compTable(double samplRate) const
{
	return m_filt_comp_table.at(samplRate);
}

double M2kAdc::convSampleToVolts(uint chnIdx, double sample) const
{
	double hw_gain = gainAt(m_chn_hw_gain_modes[chnIdx]);

	return (((sample * 0.78) / ((1 << (numAdcBits() - 1)) * 1.3 *
		hw_gain)) * m_chn_corr_gains[chnIdx] *
		m_filt_comp_table.at(sampleRate())) - m_chn_hw_offsets[chnIdx];
}

double M2kAdc::convVoltsToSample(uint chnIdx, double volts) const
{
	double hw_gain = gainAt(m_chn_hw_gain_modes[chnIdx]);

	return (volts + m_chn_hw_offsets[chnIdx]) / (m_chn_corr_gains[chnIdx] *
		m_filt_comp_table.at(sampleRate())) * ((1 << (numAdcBits() - 1))
		* 1.3 * hw_gain) / 0.78;
}

double M2kAdc::convSampleDiffToVoltsDiff(uint chnIdx, double smp) const
{
	double hw_gain = gainAt(m_chn_hw_gain_modes[chnIdx]);

	return ((smp * 0.78) / (1 << (numAdcBits() - 1)) * 1.3 *
		hw_gain) * m_chn_corr_gains[chnIdx] *
		m_filt_comp_table.at(sampleRate());
}

double M2kAdc::convVoltsDiffToSampleDiff(uint chnIdx, double v) const
{
	double hw_gain = gainAt(m_chn_hw_gain_modes[chnIdx]);

	return v / (m_chn_corr_gains[chnIdx] *
		m_filt_comp_table.at(sampleRate())) * ((1 << (numAdcBits() - 1))
		* 1.3 * hw_gain) / 0.78;
}

GenericAdc::settings_uptr M2kAdc::getCurrentHwSettings()
{
	settings_uptr gsettings_uptr = GenericAdc::getCurrentHwSettings();

	M2KSettings *m2k_settings = new M2KSettings;
	*(Settings *)m2k_settings = *gsettings_uptr;

	for (int i = 0; i < numAdcChannels(); i++) {
		m2k_settings->channel_hw_gain_mode.push_back(chnHwGainMode(i));
		m2k_settings->channel_hw_offset.push_back(chnHwOffset(i));
	}

	return std::unique_ptr<Settings>(m2k_settings);
}

void M2kAdc::setHwSettings(GenericAdc::Settings *settings)
{
	GenericAdc::setHwSettings(settings);

	M2KSettings *m2k_settings = dynamic_cast<M2KSettings *>(settings);
	if (m2k_settings) {
		for (int i = 0; i < numAdcChannels(); i++) {
			setChnHwGainMode(i,
				m2k_settings->channel_hw_gain_mode[i]);
			setChnHwOffset(i, m2k_settings->channel_hw_offset[i]);
		}
	}
}
