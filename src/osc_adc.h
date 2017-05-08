#ifndef OSC_ADC_H
#define OSC_ADC_H

#include <QList>
#include <QStringList>
#include <QPair>
#include <map>

extern "C" {
	struct iio_context;
	struct iio_device;
	struct iio_channel;
}

namespace adiscope {

class IioUtils
{
public:
	static QStringList available_options_list(struct iio_device *dev,
			const char *attr_name);
	static QList<struct iio_channel *> scan_elem_channel_list(
			struct iio_device *dev);
	static QList<struct iio_channel *> pick_channels_with_direction(
		const QList<struct iio_channel *>& list, bool output);
};

class GenericAdc
{
public:
	GenericAdc(struct iio_context *ctx, struct iio_device *adc_dev);
	virtual ~GenericAdc();

	struct iio_context * iio_context() const;
	struct iio_device * iio_adc_dev() const;
	uint numAdcChannels() const;
	QList<struct iio_channel *> adcChannelList() const;
	uint numAdcBits() const;

	double sampleRate() const;
	virtual double readSampleRate();
	virtual void setSampleRate(double);

	virtual double convSampleToVolts(uint chnIdx, double sample) const;
	virtual double convVoltsToSample(uint chnIdx, double volts) const;
	virtual double convSampleDiffToVoltsDiff(uint chnIdx, double smpl)const;
	virtual double convVoltsDiffToSampleDiff(uint chnIdx, double v) const;

private:
	struct iio_context *m_ctx;
	struct iio_device *m_adc;
	QList<struct iio_channel *> m_adc_channels;
	uint m_adc_bits;
	double m_sample_rate;
};

class M2kAdc: public GenericAdc
{
public:

	enum GainMode {
			LOW_GAIN_MODE = 0,
			HIGH_GAIN_MODE = 1,
		};

	M2kAdc(struct iio_context *, struct iio_device *adc_dev);
	~M2kAdc();

	double chnCorrectionOffset(uint chnIdx) const;
	void setChnCorrectionOffset(uint chnIdx, double offset);

	double chnCorrectionGain(uint chnIdx) const;
	void setChnCorrectionGain(uint chnIdx, double gain);

	double chnHwOffset(uint chnIdx) const;
	void setChnHwOffset(uint chnIdx, double offset);

	GainMode chnHwGainMode(uint chnIdx) const;
	void setChnHwGainMode(uint chnIdx, GainMode gain_mode);

	double gainAt(GainMode gain_mode) const;
	QPair<double, double> inputRange(GainMode gain_mode) const;

	QList<double> availSamplRates() const;
	double compTable(double samplRate) const;

	virtual double convSampleToVolts(uint chnIdx, double sample) const;
	virtual double convVoltsToSample(uint chnIdx, double volts) const;
	virtual double convSampleDiffToVoltsDiff(uint chnIdx, double smp) const;
	virtual double convVoltsDiffToSampleDiff(uint chnIdx, double v) const;

private:
	QList<struct iio_channel *> m_offset_channels;
	QList<struct iio_channel *> m_gain_channels;

	QList<double > m_availSampRates;
	std::map<double, double> m_filt_comp_table;
	QList<double> m_chn_corr_offsets;
	QList<double> m_chn_corr_gains;
	QList<double> m_chn_hw_offsets;
	QList<GainMode> m_chn_hw_gain_modes;
};

} /* namespace adiscope */

#endif // OSC_ADC_H
