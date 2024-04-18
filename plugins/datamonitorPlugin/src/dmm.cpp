#include "dmm.hpp"

#include <dmmreadstrategy.hpp>
#include <stylehelper.h>

using namespace scopy;
using namespace datamonitor;

DMM::DMM(QObject *parent)
	: QObject{parent}
{
	generateDictionaries();
}

QList<DmmDataMonitorModel *> DMM::getDmmMonitors(iio_context *ctx)
{
	QList<DmmDataMonitorModel *> result;

	auto deviceCount = iio_context_get_devices_count(ctx);
	for(int i = 0; i < deviceCount; i++) {
		iio_device *dev = iio_context_get_device(ctx, i);
		auto chnCout = iio_device_get_channels_count(dev);
		for(int j = 0; j < chnCout; j++) {
			// check if dmm
			iio_channel *chn = iio_device_get_channel(dev, j);
			// if the channel is DMM or hwmon we create a monitor for it
			if(isDMMCompatible(chn) || isHwmon(dev, chn)) {
				QString name = QString::fromStdString(iio_device_get_name(dev)) + ":" +
					QString::fromStdString(iio_channel_get_id(chn));

				DmmDataMonitorModel *channelModel =
					new DmmDataMonitorModel(name, StyleHelper::getColor("CH" + QString::number(j)));

				channelModel->setIioChannel(chn);
				channelModel->setIioDevice(dev);

				int type = iio_channel_get_type(chn);
				if(type != iio_chan_type::IIO_CHAN_TYPE_UNKNOWN) {
					DMMInfo dmmInfo = m_iioDevices.value(type);

					if(isHwmon(dev, chn)) {
						dmmInfo = m_hwmonDevices.value(type);
					}

					channelModel->setUnitOfMeasure(
						new UnitOfMeasurement(dmmInfo.key, dmmInfo.key_symbol));

					DMMReadStrategy *dmmReadStrategy = new DMMReadStrategy(dev, chn);
					dmmReadStrategy->setUmScale(dmmInfo.umScale);
					channelModel->setReadStrategy(dmmReadStrategy);

					if(iioChannelHasAttribute(chn, "offset")) {
						double offset = 0;
						iio_channel_attr_read_double(chn, "offset", &offset);

						dmmReadStrategy->setOffset(offset);
					}
				}

				result.push_back(channelModel);
			}
		}
	}

	return result;
}

bool DMM::isDMMCompatible(iio_channel *chn)
{
	// DMM channels have raw and scale and be input
	if(!iio_channel_is_output(chn) && iioChannelHasAttribute(chn, "raw") && iioChannelHasAttribute(chn, "scale")) {
		return true;
	}
	return false;
}

bool DMM::isHwmon(iio_device *dev, iio_channel *chn)
{
	if(iio_device_is_hwmon(dev) && iioChannelHasAttribute(chn, "input")) {
		auto d_name = iio_device_get_name(dev);
		auto c_name = std::string(iio_channel_get_id(chn));
		if(!c_name.empty() && d_name) {
			return true;
		}
	}

	return false;
}

bool DMM::iioChannelHasAttribute(iio_channel *chn, const std::string &attr)
{
	unsigned int nb_attr = iio_channel_get_attrs_count(chn);
	const char *attr_name;
	for(unsigned int i = 0; i < nb_attr; i++) {
		attr_name = iio_channel_get_attr(chn, i);
		std::size_t found = std::string(attr_name).find(attr);
		if(found != std::string::npos) {
			return true;
		}
	}
	return false;
}

void DMM::generateDictionaries()
{
	DMMInfo hwmon_voltage = {"Voltage", "V", 0.001};
	m_hwmonDevices.insert(hwmon_chan_type::HWMON_VOLTAGE, hwmon_voltage);

	DMMInfo hwmon_temp = {"Degree Celsius", "°C", 0.001};
	m_hwmonDevices.insert(hwmon_chan_type::HWMON_TEMP, hwmon_temp);

	DMMInfo hwmon_current = {"Ampere", "A", 0.001};
	m_hwmonDevices.insert(hwmon_chan_type::HWMON_CURRENT, hwmon_current);

	DMMInfo hwmon_power = {"milliWatt", "mW", 0.001};
	m_hwmonDevices.insert(hwmon_chan_type::HWMON_POWER, hwmon_power);

	DMMInfo hwmon_energy = {"milliJoule", "mJ", 0.001};
	m_hwmonDevices.insert(hwmon_chan_type::HWMON_ENERGY, hwmon_energy);

	DMMInfo hwmon_fan = {"Revolution/Min", "RPM"};
	m_hwmonDevices.insert(hwmon_chan_type::HWMON_FAN, hwmon_fan);

	DMMInfo hwmon_humidity = {"milli-percent", "pcm"};
	m_hwmonDevices.insert(hwmon_chan_type::HWMON_HUMIDITY, hwmon_humidity);

	DMMInfo iio_voltage = {"Voltage", "V", 0.001};
	m_iioDevices.insert(iio_chan_type::IIO_VOLTAGE, iio_voltage);

	DMMInfo iio_temp = {"Degree Celsius", "°C", 0.001};
	m_iioDevices.insert(iio_chan_type::IIO_TEMP, iio_temp);

	DMMInfo iio_current = {"Ampere", "A", 0.001};
	m_iioDevices.insert(iio_chan_type::IIO_CURRENT, iio_current);

	DMMInfo iio_pressure = {"Pascal", "Pa", 1000};
	m_iioDevices.insert(iio_chan_type::IIO_PRESSURE, iio_pressure);

	DMMInfo iio_accel = {"Metre per second squared", "m/s²"};
	m_iioDevices.insert(iio_chan_type::IIO_ACCEL, iio_accel);

	DMMInfo iio_angl_vel = {"Radian per second", "rad/s"};
	m_iioDevices.insert(iio_chan_type::IIO_ANGL_VEL, iio_angl_vel);

	DMMInfo iio_magn = {"Gauss", "Gs"};
	m_iioDevices.insert(iio_chan_type::IIO_MAGN, iio_magn);
}

QMap<int, DMMInfo> DMM::iioDevices() const { return m_iioDevices; }

QMap<int, DMMInfo> DMM::hwmonDevices() const { return m_hwmonDevices; }
