#include "datamonitormodel.hpp"

using namespace scopy;
using namespace datamonitor;

DataMonitorModel::DataMonitorModel(QString title, QColor color, UnitOfMeasurement *unitOfMeasure, QObject *parent)
	: title(title)
	, color(color)
	, unitOfMeasure(unitOfMeasure)
	, m_minValue(Q_INFINITY)
	, m_maxValue(-Q_INFINITY)
	, QObject{parent}
{
	values = new QList<QPair<uint32_t, double>>();
	m_lastReadValue = 0;
}

QString DataMonitorModel::getTitle() const { return title; }

void DataMonitorModel::setTitle(QString newTitle) { title = newTitle; }

QColor DataMonitorModel::getColor() const { return color; }

void DataMonitorModel::setColor(QColor newColor) { color = newColor; }

UnitOfMeasurement *DataMonitorModel::getUnitOfMeasure() const { return unitOfMeasure; }

void DataMonitorModel::setUnitOfMeasure(UnitOfMeasurement *newUnitOfMeasure) { unitOfMeasure = newUnitOfMeasure; }

double DataMonitorModel::getLastReadValue() const { return m_lastReadValue; }

double DataMonitorModel::getValueAtTime(uint32_t time)
{
	for(int i = 0; i < values->length(); i++) {
		if(values->at(i).first == time) {
			return values->at(i).second;
		}
	}
	return 0.0;
}

QList<QPair<uint32_t, double>> *DataMonitorModel::getValues() const { return values; }

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

void DataMonitorModel::updateValue(uint32_t time, double value)
{
	m_lastReadValue = value;
	values->append(qMakePair(time, value));
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
