#include "readabledatamonitormodel.hpp"

#include <pluginbase/preferences.h>

using namespace scopy;
using namespace datamonitor;

ReadableDataMonitorModel::ReadableDataMonitorModel(QObject *parent) {}

ReadableDataMonitorModel::ReadableDataMonitorModel(QString name, QColor color, UnitOfMeasurement *unitOfMeasure,
						   IReadStrategy *readStrategy, QObject *parent)
	: m_readStrategy(readStrategy)
	, DataMonitorModel{parent}
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
}

void ReadableDataMonitorModel::read()
{
	if(m_readStrategy) {
		m_readStrategy->read();
	}
}

IReadStrategy *ReadableDataMonitorModel::readStrategy() const { return m_readStrategy; }

void ReadableDataMonitorModel::setReadStrategy(IReadStrategy *newReadStrategy)
{
	m_readStrategy = newReadStrategy;
	connect(m_readStrategy, &IReadStrategy::readDone, this, &ReadableDataMonitorModel::addValue);
}

void ReadableDataMonitorModel::resetMinMax()
{
	setMinValue(Q_INFINITY);
	setMaxValue(-Q_INFINITY);
}
