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
#ifndef HARDWARE_TRIGGER_H
#define HARDWARE_TRIGGER_H

#include <iio.h>
#include <QList>
#include <QVector>
#include <QString>
#include <memory>

extern "C" {
	struct iio_device;
	struct iio_channel;
}

namespace adiscope {

class HardwareTrigger
{
public:
	enum condition {
		RISING_EDGE = 0,
		FALLING_EDGE = 1,
		LOW = 3,
		HIGH = 4,
		ANY_EDGE = 5,
	};

	enum mode {
			ALWAYS = 0,
			ANALOG = 1,
			DIGITAL = 2,
			DIGITAL_OR_ANALOG = 3,
			DIGITAL_AND_ANALOG = 4,
			DIGITAL_XOR_ANALOG = 5,
			TRIGGER_IN = 6,
			ANALOG_OR_TRIGGER_IN = 7,
			DIGITAL_OR_TRIGGER_IN = 8,
			DIGITAL_OR_ANALOG_OR_TRIGGER_IN = 9,
	};

	enum direction {
		TRIGGER_INPUT = 0,
		TRIGGER_OUT = 1,
	};

	enum out_select {
		sw_trigger = 0,
		trigger_i_same_channel = 1,
		trigger_i_swap_channel = 2,
		trigger_adc = 3,
		trigger_in = 4,
	};

	struct Settings {
		QList<condition> analog_condition;
		QList<condition> digital_condition;
		QList<int> level;
		QList<int> hysteresis;
		QList<enum mode> mode;
		QString source;
		int delay;
	};

	typedef std::unique_ptr<HardwareTrigger::Settings> settings_uptr;

public:

	HardwareTrigger(struct iio_device *trigger_dev);

	uint numChannels() const;

	condition analogCondition(uint chnIdx) const;
	void setAnalogCondition(uint chnIdx, condition cond);

	condition digitalCondition(uint chnIdx) const;
	void setDigitalCondition(uint chnIdx, condition cond);

	int level(uint chnIdx) const;
	void setLevel(uint chnIdx, int level);

	int hysteresis(uint chnIdx) const;
	void setHysteresis(uint chnIdx, int histeresis);

	mode triggerMode(uint chnIdx) const;
	void setTriggerMode(uint chnIdx, mode mode);

	QString source() const;
	void setSource(const QString& source);

	int sourceChannel() const;
	void setSourceChannel(uint chnIdx, bool intern_checked, bool extern_trigger_in_checked);

	bool triggerIn() const;
	void setTriggerIn(bool bo);

	int delay() const;
	void setDelay(int delay);

	std::unique_ptr<struct Settings> getCurrentHwSettings();
	void setHwTriggerSettings(struct Settings *settings);

	void setStreamingFlag(bool);
	bool getStreamingFlag();

	bool hasExternalTriggerIn() const;
	bool hasExternalTriggerOut() const;
	bool hasCrossInstrumentTrigger() const;
	void setExternalDirection(uint chnIdx, direction dir);
	void setExternalOutSelect(uint chnIdx, out_select out);


private:
	struct iio_device *m_trigger_device;
	QList<struct iio_channel *> m_analog_channels;
	QList<struct iio_channel *> m_digital_channels;
	QList<struct iio_channel *> m_logic_channels;
	struct iio_channel *m_delay_trigger;
	bool m_streaming_flag;
	bool m_trigger_in;

	uint m_num_channels;

	const static QVector<QString> lut_analog_trigg_cond;
	const static QVector<QString> lut_digital_trigg_cond;
	const static QVector<QString> lut_digital_out_direction;
	const static QVector<QString> lut_digital_out_select;
	const static QVector<QString> lut_trigg_mode;
	const static QVector<QString> lut_trigg_source;
};

} /* namespace adiscope */

#endif // HARDWARE_TRIGGER_H
