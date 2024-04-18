#include "dmmdatamonitormodel.hpp"

#include <pluginbase/preferences.h>

using namespace scopy;
using namespace datamonitor;

DmmDataMonitorModel::DmmDataMonitorModel(QObject *parent) {}

DmmDataMonitorModel::DmmDataMonitorModel(QString name, QColor color, UnitOfMeasurement *unitOfMeasure,
					 DMMReadStrategy *readStrategy, QObject *parent)
	: ReadableDataMonitorModel(parent)
{
	setName(name);
	setColor(color);

	if(unitOfMeasure) {
		setUnitOfMeasure(unitOfMeasure);
	} else {
		setUnitOfMeasure(new UnitOfMeasurement("Volt", "V"));
	}

	Preferences *p = Preferences::GetInstance();
	QObject::connect(p, &Preferences::preferenceChanged, this, [=, this](QString id, QVariant var) {
		if(id.contains("datamonitor")) {
			setDataStorageSize();
		}
	});

	setDataStorageSize();
	setReadStrategy(readStrategy);
}

iio_channel *DmmDataMonitorModel::iioChannel() const { return m_iioChannel; }

void DmmDataMonitorModel::setIioChannel(iio_channel *newIioChannel) { m_iioChannel = newIioChannel; }

QString DmmDataMonitorModel::getDeviceName() { return QString::fromStdString(iio_device_get_name(m_iioDevice)); }

iio_device *DmmDataMonitorModel::iioDevice() const { return m_iioDevice; }

void DmmDataMonitorModel::setIioDevice(iio_device *newIioDevice) { m_iioDevice = newIioDevice; }
