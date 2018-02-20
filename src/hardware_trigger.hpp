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
		N_DIGITAL_OR_ANALOG = 6,
		N_DIGITAL_AND_ANALOG = 7,
		N_DIGITAL_XOR_ANALOG = 8,
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
	void setSourceChannel(uint chnIdx);

	int delay() const;
	void setDelay(int delay);

	std::unique_ptr<struct Settings> getCurrentHwSettings();
	void setHwTriggerSettings(struct Settings *settings);

	void setStreamingFlag(bool);
	bool getStreamingFlag();

private:
	struct iio_device *m_trigger_device;
	QList<struct iio_channel *> m_analog_channels;
	QList<struct iio_channel *> m_digital_channels;
	QList<struct iio_channel *> m_logic_channels;
	struct iio_channel *m_delay_trigger;
	bool m_streaming_flag;

	uint m_num_channels;

	static QVector<QString> lut_analog_trigg_cond;
	static QVector<QString> lut_digital_trigg_cond;
	static QVector<QString> lut_trigg_mode;
	static QVector<QString> lut_trigg_source;
};

} /* namespace adiscope */

#endif // HARDWARE_TRIGGER_H
