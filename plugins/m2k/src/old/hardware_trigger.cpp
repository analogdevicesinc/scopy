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
#include "hardware_trigger.hpp"
#include <QPair>

#include <stdexcept>

using namespace adiscope;

QVector<QString> HardwareTrigger::lut_analog_trigg_cond = {
	"edge-rising",
	"edge-falling",
	"level-low",
	"level-high",
};

QVector<QString> HardwareTrigger::lut_digital_trigg_cond = {
	"edge-rising",
	"edge-falling",
	"level-low",
	"level-high",
	"edge-any",
};

QVector<QString> HardwareTrigger::lut_trigg_mode = {
	"always",
	"analog",
	"digital",
	"digital_OR_analog",
	"digital_AND_analog",
	"digital_XOR_analog",
	"!digital_OR_analog",
	"!digital_AND_analog",
	"!digital_XOR_analog",
};

QVector<QString> HardwareTrigger::lut_digital_out_direction = {
	"in",
	"out"
};

QVector<QString> HardwareTrigger::lut_digital_out_select= {
	"sw-trigger",
	"trigger-i-same-chn",
	"trigger-i-swap-chn",
	"trigger-adc",
	"trigger-in",
};

// This is still not generic. It is specific to only 2 channels!
QVector<QString> HardwareTrigger::lut_trigg_source = {
	"a",
	"b",
	"a_OR_b",
	"a_AND_b",
	"a_XOR_b",
	"trigger_in",
	"a_OR_trigger_in",
	"b_OR_trigger_in",
	"a_OR_b_OR_trigger_in",

};

typedef QPair<struct iio_channel *, QString> channel_pair;

HardwareTrigger::HardwareTrigger(struct iio_device *trigg_dev) :
	m_trigger_device(trigg_dev)
{
	if (!trigg_dev) {
		throw std::invalid_argument("trigger_device=NULL");
	}

	// Get all channels and sort them ascending by name
	QList<QPair<struct iio_channel *, QString>> channels;
	for (uint i = 0; i < iio_device_get_channels_count(trigg_dev); i++) {
		struct iio_channel *chn = iio_device_get_channel(trigg_dev, i);

		if (iio_channel_is_output(chn)) {
			continue;
		}

		QString name = QString(iio_channel_get_id(chn));
		QPair<struct iio_channel *, QString> chn_pair(chn, name);
		channels.push_back(chn_pair);
	}
	std::sort(channels.begin(), channels.end(),
		[](channel_pair a, channel_pair b)
			{ return a.second < b.second; });

	// Pick the analog, digital, trigger_logic and delay channels
	for (int i = 0; i < channels.size(); i++) {
		struct iio_channel *chn = channels[i].first;
		bool mode = iio_channel_find_attr(chn, "mode");
		bool trigger = iio_channel_find_attr(chn, "trigger");
		bool trigger_level = iio_channel_find_attr(chn,
				"trigger_level");
		bool trigger_hysteresis = iio_channel_find_attr(chn,
				"trigger_hysteresis");

		if (trigger) {
			if (trigger_level && trigger_hysteresis) {
				m_analog_channels.push_back(chn);
			} else if (!trigger_level && !trigger_hysteresis) {
				m_digital_channels.push_back(chn);
			}
		} else if (mode) {
			m_logic_channels.push_back(chn);
		}
	}

	m_delay_trigger = iio_device_find_channel(trigg_dev, "trigger",
			false);

	m_num_channels = m_analog_channels.size();

	if (m_analog_channels.size() < 1) {
		throw std::runtime_error(
			"hardware trigger has no analog channels");
	}

	if (m_digital_channels.size() < 1) {
		throw std::runtime_error(
			"hardware trigger has no digital channels");
	}

	if (m_logic_channels.size() < 1) {
		throw std::runtime_error(
			"hardware trigger has no trigger_logic channels");
	}

	if (!m_delay_trigger) {
		throw std::runtime_error("no delay trigger available");
	}


	setStreamingFlag(false);
}

uint HardwareTrigger::numChannels() const
{
	return m_num_channels;
}

HardwareTrigger::condition HardwareTrigger::analogCondition(uint chnIdx)
		const
{
	if (chnIdx >= numChannels()) {
		throw std::invalid_argument("Channel index is out of range");
	}

	ssize_t ret;
	char buf[4096];

	ret = iio_channel_attr_read(m_analog_channels[chnIdx], "trigger", buf,
		sizeof(buf));
	if (ret < 0) {
		throw std::runtime_error("failed to read attribute: trigger");
	}
	auto it = std::find(lut_analog_trigg_cond.begin(),
		lut_analog_trigg_cond.end(), buf);
	if  (it == lut_analog_trigg_cond.end()) {
		throw std::runtime_error(
			"unexpected value read from attribute: trigger");
	}

	return static_cast<condition>(it - lut_analog_trigg_cond.begin());

}

void HardwareTrigger::setAnalogCondition(uint chnIdx, condition cond)
{
	if (chnIdx >= numChannels()) {
		throw std::invalid_argument("Channel index is out of range");
	}

	if (cond == ANY_EDGE) {
		return; //or throw?
	}

	QByteArray byteArray =  lut_analog_trigg_cond[cond].toLatin1();
	iio_channel_attr_write(m_analog_channels[chnIdx], "trigger",
		byteArray.data());
}

HardwareTrigger::condition HardwareTrigger::digitalCondition(uint chnIdx)
		const
{
	if (chnIdx >= numChannels()) {
		throw std::invalid_argument("Channel index is out of range");
	}

	ssize_t ret;
	char buf[4096];

	ret = iio_channel_attr_read(m_digital_channels[chnIdx], "trigger", buf,
		sizeof(buf));
	if (ret < 0)
		throw "failed to read attribute: trigger";
	auto it = std::find(lut_digital_trigg_cond.begin(),
		lut_digital_trigg_cond.end(), buf);
	if  (it == lut_digital_trigg_cond.end()) {
		throw "unexpected value read from attribute: trigger";
	}

	return static_cast<condition>(it - lut_digital_trigg_cond.begin());
}

bool HardwareTrigger::hasExternalTriggerOut() const
{
	auto ret = iio_channel_find_attr(m_logic_channels[1], "out_select");
	return (ret==NULL) ? false : true;
}

bool HardwareTrigger::hasExternalTriggerIn() const
{
	return true;
}

bool HardwareTrigger::hasCrossInstrumentTrigger() const
{
	auto ret = iio_channel_find_attr(m_logic_channels[1], "out_select");
	return (ret==NULL) ? false : true;
}

void HardwareTrigger::setExternalDirection(uint chnIdx, direction dir)
{
	if(hasExternalTriggerOut()) {
	if (chnIdx >= numChannels()) {
		throw std::invalid_argument("Channel index is out of range");
	}

	QByteArray byteArray =  lut_digital_out_direction[dir].toLatin1();
	iio_channel_attr_write(m_logic_channels[chnIdx], "out_direction",
		byteArray.data());
	}
}

void HardwareTrigger::setExternalOutSelect(uint chnIdx, out_select out)
{
	if(hasExternalTriggerOut()) {
	if (chnIdx >= numChannels()) {
		throw std::invalid_argument("Channel index is out of range");
	}

	QByteArray byteArray =  lut_digital_out_select[out].toLatin1();
	iio_channel_attr_write(m_logic_channels[chnIdx], "out_select",
		byteArray.data());
	}
}

void HardwareTrigger::setDigitalCondition(uint chnIdx, condition cond)
{
	if (chnIdx >= numChannels()) {
		throw std::invalid_argument("Channel index is out of range");
	}

	QByteArray byteArray =  lut_digital_trigg_cond[cond].toLatin1();
	iio_channel_attr_write(m_digital_channels[chnIdx], "trigger",
		byteArray.data());
}

int HardwareTrigger::level(uint chnIdx) const
{
	if (chnIdx >= numChannels()) {
		throw std::invalid_argument("Channel index is out of range");
	}

	long long val;

	iio_channel_attr_read_longlong(m_analog_channels[chnIdx],
		"trigger_level", &val);

	return static_cast<int>(val);
}

void HardwareTrigger::setLevel(uint chnIdx, int level)
{
	if (chnIdx >= numChannels()) {
		throw std::invalid_argument("Channel index is out of range");
	}

	iio_channel_attr_write_longlong(m_analog_channels[chnIdx],
		"trigger_level", static_cast<long long> (level));
}

int HardwareTrigger::hysteresis(uint chnIdx) const
{
	if (chnIdx >= numChannels()) {
		throw std::invalid_argument("Channel index is out of range");
	}

	long long val;

	iio_channel_attr_read_longlong(m_analog_channels[chnIdx],
		"trigger_hysteresis", &val);

	return static_cast<int>(val);
}

void HardwareTrigger::setHysteresis(uint chnIdx, int histeresis)
{
	if (chnIdx >= numChannels()) {
		throw std::invalid_argument("Channel index is out of range");
	}

	iio_channel_attr_write_longlong(m_analog_channels[chnIdx],
		"trigger_hysteresis", static_cast<long long>(histeresis));
}

HardwareTrigger::mode HardwareTrigger::triggerMode(uint chnIdx) const
{
	if (chnIdx >= numChannels()) {
		throw std::invalid_argument("Channel index is out of range");
	}

	ssize_t ret;
	char buf[4096];

	ret = iio_channel_attr_read(m_logic_channels[chnIdx], "mode", buf,
		sizeof(buf));
	if (ret < 0) {
		throw ("failed to read attribute: mode");
	}
	auto it = std::find(lut_trigg_mode.begin(),
		lut_trigg_mode.end(), buf);
	if  (it == lut_trigg_mode.end()) {
		throw ("unexpected value read from attribute: mode");
	}

	return static_cast<mode>(it - lut_trigg_mode.begin());
}

void HardwareTrigger::setTriggerMode(uint chnIdx, HardwareTrigger::mode mode)
{
	if (chnIdx >= numChannels()) {
		throw std::invalid_argument("Channel index is out of range");
	}

	QByteArray byteArray =  lut_trigg_mode[mode].toLatin1();
	iio_channel_attr_write(m_logic_channels[chnIdx], "mode",
		byteArray.data());
}

QString HardwareTrigger::source() const
{
	char buf[4096];

	iio_channel_attr_read(m_delay_trigger, "logic_mode", buf, sizeof(buf));

	return QString(buf);
}

void HardwareTrigger::setSource(const QString& source)
{
	QByteArray byteArray = source.toLatin1();
	iio_channel_attr_write(m_delay_trigger, "logic_mode",
			       byteArray.data());
}

/*
 * Convenience function to be used when willing to use the trigger for only one
 * channel at a time.
 */

bool HardwareTrigger::triggerIn() const
{
	return m_trigger_in;
}

void HardwareTrigger::setTriggerIn(bool bo)
{
	m_trigger_in = bo;
}

int HardwareTrigger::sourceChannel() const
{
	int chnIdx = -1;

	QString mode = source();

	// Returning the channel index if a single channel is set as a source
	// and -1 if multiple channels are set.
	if (mode.length() == 1) {
		chnIdx = mode.at(0).digitValue() - QChar('a').digitValue();
	}

	return chnIdx;
}

/*
 * Convenience function to be used when willing to enable the trigger for only
 * one channel at a time.
 */
void HardwareTrigger::setSourceChannel(uint chnIdx, bool intern_checked, bool extern_trigger_in_checked)
{
	if (chnIdx >= numChannels()) {
		throw std::invalid_argument("Channel index is out of range");
	}

	// Currently we don't need trigger on multiple channels simultaneously
	// Also options 'a_OR_b', 'a_AND_b' and 'a_XOR_b' don't scale well for
	// 3, 4, .. channels (combinations rise exponentially).
	QChar chn('a' + chnIdx);
	QString source = QString(chn);
	if(m_trigger_in) {
		if(!intern_checked && !extern_trigger_in_checked)
			source = "trigger_in";
		else
			source = source + "_OR_trigger_in";
	}
	setSource(source);
}

int HardwareTrigger::delay() const
{
	long long delay;

	iio_channel_attr_read_longlong(m_delay_trigger, "delay", &delay);

	return static_cast<int>(delay);
}

void HardwareTrigger::setDelay(int delay)
{
	iio_channel_attr_write_longlong(m_delay_trigger, "delay", delay);
}

void HardwareTrigger::setStreamingFlag(bool val)
{
	m_streaming_flag = val;
	iio_device_attr_write_bool(m_trigger_device, "streaming", val);
}

bool HardwareTrigger::getStreamingFlag()
{
	return m_streaming_flag;
}

HardwareTrigger::settings_uptr HardwareTrigger::getCurrentHwSettings()
{
	settings_uptr settings(new Settings);

	for (uint i = 0; i < numChannels(); i++) {
		settings->analog_condition.push_back(analogCondition(i));
		settings->digital_condition.push_back(digitalCondition(i));
		settings->level.push_back(level(i));
		settings->hysteresis.push_back(hysteresis(i));
		settings->mode.push_back(triggerMode(i));
		settings->source = source();
		settings->delay = delay();
	}

	return settings;
}

void HardwareTrigger::setHwTriggerSettings(struct Settings *settings)
{
	for (uint i = 0; i < numChannels(); i++) {
		setAnalogCondition(i, settings->analog_condition[i]);
		setDigitalCondition(i, settings->digital_condition[i]);
		setLevel(i, settings->level[i]);
		setHysteresis(i, settings->hysteresis[i]);
		setTriggerMode(i, settings->mode[i]);
		setSource(settings->source);
		setDelay(settings->delay);
	}
}
