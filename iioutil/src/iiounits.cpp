#include <iiounits.h>
#include <QApplication>

using namespace scopy;

IIOUnitsManager *IIOUnitsManager::pinstance_{nullptr};

IIOUnitsManager::IIOUnitsManager(QObject *parent)
	: QObject(parent)
{

	m_iioChannelTypes = QMap<iio_chan_type, IIOUnit>({
		{IIO_VOLTAGE, {"Voltage", "V", 0.001}},
		{IIO_TEMP, {"Degree Celsius", "°C", 0.001}},
		{IIO_CURRENT, {"Ampere", "A", 0.001}},
		{IIO_PRESSURE, {"Pascal", "Pa", 1000}},
		{IIO_ACCEL, {"Metre per second squared", "m/s2", 1}},
		{IIO_ANGL_VEL, {"Radian per second", "rad/s", 1}},
		{IIO_MAGN, {"Gauss", "Gs", 1}}
		/// and others
	});

	m_hwmonChannelTypes = {
		{HWMON_VOLTAGE, {"Voltage", "V", 0.001}}, {HWMON_TEMP, {"Degree Celsius", "°C", 0.001}},
		{HWMON_CURRENT, {"Ampere", "A", 0.001}},  {HWMON_POWER, {"Watt", "W", 0.000001}},
		{HWMON_ENERGY, {"Joule", "J", 0.000001}}, {HWMON_FAN, {"Revolution/Min", "RPM", 1}},
		{HWMON_HUMIDITY, {"percent", "%", 0.001}}
		/// and others
	};
}

IIOUnitsManager *IIOUnitsManager::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new IIOUnitsManager(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

QMap<iio_chan_type, IIOUnit> IIOUnitsManager::iioChannelTypes()
{
	return IIOUnitsManager::GetInstance()->_iioChannelTypes();
}

QMap<hwmon_chan_type, IIOUnit> IIOUnitsManager::hwmonChannelTypes()
{
	return IIOUnitsManager::GetInstance()->_hwmonChannelTypes();
}

IIOUnitsManager::~IIOUnitsManager() {}

QMap<iio_chan_type, IIOUnit> IIOUnitsManager::_iioChannelTypes() { return m_iioChannelTypes; }

QMap<hwmon_chan_type, IIOUnit> IIOUnitsManager::_hwmonChannelTypes() { return m_hwmonChannelTypes; }

#include "moc_iiounits.cpp"
