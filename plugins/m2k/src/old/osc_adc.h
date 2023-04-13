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
#ifndef OSC_ADC_H
#define OSC_ADC_H

#include <QList>
#include <QStringList>
#include <QPair>
#include <map>
#include <memory>

extern "C" {
	struct iio_context;
	struct iio_device;
	struct iio_channel;
}

namespace scopy {

class HardwareTrigger;

class IioUtils
{
public:
	static QStringList available_options_list(struct iio_device *dev,
			const char *attr_name);
	static QList<struct iio_channel *> scan_elem_channel_list(
			struct iio_device *dev);
	static QList<struct iio_channel *> pick_channels_with_direction(
		const QList<struct iio_channel *>& list, bool output);
	static std::string hardware_revision(struct iio_context *);
};

class GenericAdc
{
public:
	struct Settings {
		double sample_rate;

		virtual ~Settings() {}
	};

	typedef std::unique_ptr<GenericAdc::Settings> settings_uptr;

public:
	GenericAdc(struct iio_context *ctx, struct iio_device *adc_dev);
	virtual ~GenericAdc();

	struct iio_context * iio_context() const;
	struct iio_device * iio_adc_dev() const;
	std::shared_ptr<HardwareTrigger> getTrigger() const;

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

	virtual std::unique_ptr<Settings> getCurrentHwSettings();
	virtual void setHwSettings(Settings *settings);

protected:
	std::shared_ptr<HardwareTrigger> m_trigger;
	struct iio_context *m_ctx;
	struct iio_device *m_adc;
	QList<struct iio_channel *> m_adc_channels;


private:
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

	struct M2KSettings: public Settings {
		QList<GainMode> channel_hw_gain_mode;
		QList<double> channel_hw_offset;
	};

public:

	M2kAdc(struct iio_context *, struct iio_device *adc_dev);
	~M2kAdc();

	void apply_m2k_fixes();

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

	void setFilteringEnabled(bool set);
	bool filteringEnabled() const;

	uint32_t oversamplingRatio() const;
	void setOversamplingRatio(uint32_t ratio) const;

	QList<double> availSamplRates() const;
	double maxSampleRate() const;
	double compTable(double samplRate) const;
	void setSampleRate(double);
	double readSampleRate() const;

	virtual double convSampleToVolts(uint chnIdx, double sample) const;
	virtual double convVoltsToSample(uint chnIdx, double volts) const;
	virtual double convSampleDiffToVoltsDiff(uint chnIdx, double smp) const;
	virtual double convVoltsDiffToSampleDiff(uint chnIdx, double v) const;

	virtual std::unique_ptr<struct Settings> getCurrentHwSettings();
	virtual void setHwSettings(struct Settings *settings);

private:
	QList<struct iio_channel *> m_offset_channels;
	QList<struct iio_channel *> m_gain_channels;

	QList<double > m_availSampRates;
	std::map<double, double> m_filt_comp_table;
	QList<double> m_chn_corr_offsets;
	QList<double> m_chn_corr_gains;
	QList<double> m_chn_hw_offsets;
	QList<GainMode> m_chn_hw_gain_modes;
	bool filtering_enabled;
	double max_sample_rate;
	double m2k_sample_rate;

};

class AdcBuilder
{
public:
	enum AdcType {
		GENERIC = 0,
		M2K = 1,
	};

	static std::shared_ptr<GenericAdc> newAdc(AdcType adc_type,
		struct iio_context *ctx, struct iio_device *adc)
	{
		switch (adc_type) {
		case GENERIC: return std::make_shared<GenericAdc>(ctx, adc);
		case M2K: return std::make_shared<M2kAdc>(ctx, adc);
		}
		return nullptr;
	}
};

} /* namespace scopy */

#endif // OSC_ADC_H
