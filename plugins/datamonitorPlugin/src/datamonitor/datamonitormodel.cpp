#include "datamonitormodel.hpp"

#include <pluginbase/preferences.h>
#include <QDateTime>
#include <QDebug>
#include <QwtDate>

using namespace scopy;
using namespace datamonitor;

DataMonitorModel::DataMonitorModel(QString name, QColor color, UnitOfMeasurement *unitOfMeasure, QObject *parent)
	: name(name)
	, color(color)
	, m_minValue(Q_INFINITY)
	, m_maxValue(-Q_INFINITY)
	, QObject{parent}
{
	if(unitOfMeasure) {
		this->unitOfMeasure = unitOfMeasure;
	} else {
		this->unitOfMeasure = new UnitOfMeasurement("Volt", "V");
	}

	Preferences *p = Preferences::GetInstance();
	QObject::connect(p, &Preferences::preferenceChanged, this, [=, this](QString id, QVariant var) {
		if(id.contains("datamonitor")) {
			setDataStorageSize();
		}
	});

	setDataStorageSize();
}

QString DataMonitorModel::getName() const { return name; }

void DataMonitorModel::setName(QString newName) { name = newName; }

QColor DataMonitorModel::getColor() const { return color; }

void DataMonitorModel::setColor(QColor newColor) { color = newColor; }

UnitOfMeasurement *DataMonitorModel::getUnitOfMeasure() const { return unitOfMeasure; }

void DataMonitorModel::setUnitOfMeasure(UnitOfMeasurement *newUnitOfMeasure) { unitOfMeasure = newUnitOfMeasure; }

QPair<double, double> DataMonitorModel::getLastReadValue() const { return qMakePair(xdata.last(), ydata.last()); }

double DataMonitorModel::getValueAtTime(double time)
{
	if(xdata.contains(time)) {
		return ydata.at(xdata.indexOf(time));
	}

	return 0.0;
}

QList<QPair<double, double>> *DataMonitorModel::getValues() const
{
	QList<QPair<double, double>> *result = new QList<QPair<double, double>>();

	for(int i = 0; i < xdata.length(); i++) {
		result->push_back(qMakePair(xdata.at(i), ydata.at(i)));
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

IReadStrategy *DataMonitorModel::getReadStrategy() const { return readStrategy; }

void DataMonitorModel::setDataStorageSize()
{
	Preferences *p = Preferences::GetInstance();

	auto dataSizePref = p->get("datamonitorplugin_data_storage_size").toString().split(" ");
	if(dataSizePref[0].toDouble() == 0) {
		// set default value if nothing is found in preferences
		m_dataSize = 1000;
	} else {
		m_dataSize = dataSizePref[0].toDouble();
		if(dataSizePref[1] == "Kb") {
			m_dataSize *= 1000;
		} else if(dataSizePref[1] == "Mb") {
			m_dataSize *= 1000000;
		}
	}
}

void DataMonitorModel::setReadStrategy(IReadStrategy *newReadStrategy)
{
	readStrategy = newReadStrategy;
	connect(readStrategy, &IReadStrategy::readDone, this, &DataMonitorModel::updateValue);
}

QVector<double> *DataMonitorModel::getXdata() { return &xdata; }

QVector<double> *DataMonitorModel::getYdata() { return &ydata; }

void DataMonitorModel::clearMonitorData()
{
	xdata.erase(xdata.begin(), xdata.end());
	ydata.erase(ydata.begin(), ydata.end());
	resetMinMax();
	Q_EMIT dataCleared();
}

void DataMonitorModel::read() { readStrategy->read(); }

double DataMonitorModel::minValue() const { return m_minValue; }

double DataMonitorModel::maxValue() const { return m_maxValue; }

void DataMonitorModel::updateValue(double time, double value)
{
	// make sure the total amout of data won't be more than what is set in prefferences
	if(xdata.length() >= m_dataSize) {
		xdata.pop_front();
		ydata.pop_front();
	}

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
