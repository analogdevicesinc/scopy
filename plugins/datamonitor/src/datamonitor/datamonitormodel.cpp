#include "datamonitormodel.hpp"

#include <pluginbase/preferences.h>

using namespace scopy;
using namespace datamonitor;

DataMonitorModel::DataMonitorModel(QString name, QColor color, UnitOfMeasurement *unitOfMeasure, QObject *parent)
	: name(name)
	, color(color)
	, unitOfMeasure(unitOfMeasure)
	, m_minValue(Q_INFINITY)
	, m_maxValue(-Q_INFINITY)
	, QObject{parent}
{
	//TODO RESERVE SPACE FOR X AND Y DATA
	Preferences *p = Preferences::GetInstance();
	QObject::connect(p, &Preferences::preferenceChanged, this, [=](QString id, QVariant var) {
		if (id.contains("datamonitor")) {
			auto dataSizePref = p->get("datamonitor_data_storage_size").toString().split(" ");
			int dataSize = dataSizePref[0].toInt();
			if (dataSizePref[1] == "Kb") {
				dataSize *= 1000;
			} else if (dataSizePref[1] == "Mb") {
				dataSize *= 1000000;
			}

			//if any data stored should we delete it ?
			//?? can user update while program is running (between runs )
			xdata.reserve(dataSize);
			ydata.reserve(dataSize);
		}
	});
}

QString DataMonitorModel::getName() const { return name; }

void DataMonitorModel::setName(QString newName) { name = newName; }

QColor DataMonitorModel::getColor() const { return color; }

void DataMonitorModel::setColor(QColor newColor) { color = newColor; }

UnitOfMeasurement *DataMonitorModel::getUnitOfMeasure() const { return unitOfMeasure; }

void DataMonitorModel::setUnitOfMeasure(UnitOfMeasurement *newUnitOfMeasure) { unitOfMeasure = newUnitOfMeasure; }

QPair<double,double> DataMonitorModel::getLastReadValue() const { return qMakePair(xdata.last(), ydata.last()); }

double DataMonitorModel::getValueAtTime(double time)
{
	if (ydata.contains(time)) {
		return xdata.at(ydata.indexOf(time));
	}

	return 0.0;
}

QList<QPair<double, double>> *DataMonitorModel::getValues() const {
	QList<QPair<double, double>> *result = new QList<QPair<double, double>>();

	for( int i = 0; i < xdata.length(); i++) {
		result->push_back(qMakePair(xdata.at(i),ydata.at(i)));
	}

	return result;
}

void DataMonitorModel::checkMinMaxUpdate(double value)
{
	if(value < m_minValue) {
		m_minValue = value;
		Q_EMIT minValueUpdated(value);
	}
	if(value > m_maxValue) {
		m_maxValue = value;
		Q_EMIT maxValueUpdated(value);
	}
}

IReadStrategy *DataMonitorModel::getReadStrategy() const
{
	return readStrategy;
}

void DataMonitorModel::setReadStrategy(IReadStrategy *newReadStrategy)
{
	readStrategy = newReadStrategy;

	connect(readStrategy, &IReadStrategy::readDone, this, &DataMonitorModel::updateValue);
}

void DataMonitorModel::clearMonitorData()
{
	xdata.erase(xdata.begin(), xdata.end());
	ydata.erase(ydata.begin(), ydata.end());
	resetMinMax();
	Q_EMIT dataCleared();
}

void DataMonitorModel::read()
{
	readStrategy->read();
}

void DataMonitorModel::updateValue(double time, double value)
{
	xdata.push_back(time);
	ydata.push_back(value);
	checkMinMaxUpdate(value);

	Q_EMIT valueUpdated(time, value);
}

void DataMonitorModel::resetMinMax()
{
	m_minValue = Q_INFINITY;
	Q_EMIT minValueUpdated(m_minValue);
	m_maxValue = -Q_INFINITY;
	Q_EMIT maxValueUpdated(m_maxValue);
}
