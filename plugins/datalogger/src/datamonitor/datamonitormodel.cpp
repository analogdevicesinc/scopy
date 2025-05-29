/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "datamonitor/datamonitormodel.hpp"

#include <pluginbase/preferences.h>
#include <QDateTime>
#include <QDebug>
#include <QwtDate>

using namespace scopy;
using namespace datamonitor;

DataMonitorModel::DataMonitorModel(QObject *parent, double defaultScale, double defaultOffset)
	: m_defaultScale(defaultScale)
	, m_defaultOffset(defaultOffset)
	, m_scale(defaultScale)
	, m_offset(defaultOffset)
{
	setDataStorageSize();
}

DataMonitorModel::DataMonitorModel(QString name, QColor color, UnitOfMeasurement *unitOfMeasure, double defaultScale,
				   double defaultOffset, QObject *parent)
	: color(color)
	, m_defaultScale(defaultScale)
	, m_defaultOffset(defaultOffset)
	, m_scale(defaultScale)
	, m_offset(defaultOffset)
	, QObject{parent}
{
	m_minValue = std::numeric_limits<double>::max();
	m_maxValue = std::numeric_limits<double>::min();
	setName(name);

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

void DataMonitorModel::setName(QString newName)
{
	name = newName;
	auto nameList = name.split(":");
	setDeviceName(nameList[0]);
	setShortName(nameList[1]);
}

QColor DataMonitorModel::getColor() const { return color; }

void DataMonitorModel::setColor(QColor newColor) { color = newColor; }

UnitOfMeasurement *DataMonitorModel::getUnitOfMeasure() const { return unitOfMeasure; }

void DataMonitorModel::setUnitOfMeasure(UnitOfMeasurement *newUnitOfMeasure) { unitOfMeasure = newUnitOfMeasure; }

QPair<double, double> DataMonitorModel::getLastReadValue() const
{

	if(xdata.isEmpty()) {
		return qMakePair(0, 0);
	}

	return qMakePair(xdata.last(), ydata.last());
}

double DataMonitorModel::getValueAtTime(double time)
{
	if(xdata.contains(time)) {
		return ydata.at(xdata.indexOf(time));
	}

	return -Q_INFINITY;
}

void DataMonitorModel::setValueAtTime(double time, double value)
{
	if(xdata.contains(time)) {
		ydata.replace(xdata.indexOf(time), value);
	} else {
		addValue(time, value);
	}
}

void DataMonitorModel::checkMinMaxUpdate(double value)
{
	if(value < m_minValue) {
		setMinValue(value);
	}
	if(value > m_maxValue) {
		setMaxValue(value);
	}
}

uint32_t DataMonitorModel::plotUuid() const { return m_plotUuid; }

void DataMonitorModel::setPlotUuid(uint32_t newPlotUuid) { m_plotUuid = newPlotUuid; }

void DataMonitorModel::setDataStorageSize()
{
	Preferences *p = Preferences::GetInstance();

	auto dataSizePref = p->get("dataloggerplugin_data_storage_size").toString().split(" ");
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

void DataMonitorModel::setHasScale(bool newHasScale) { m_hasScale = newHasScale; }

bool DataMonitorModel::hasScale() const { return m_hasScale; }

bool DataMonitorModel::hasOffset() const { return m_hasOffset; }

void DataMonitorModel::setHasOffset(bool newHasOffset) { m_hasOffset = newHasOffset; }

double DataMonitorModel::defaultOffset() const { return m_defaultOffset; }

void DataMonitorModel::setYdata(const QVector<double> &newYdata)
{
	ydata.erase(ydata.begin(), ydata.end());
	ydata.append(newYdata);
	Q_EMIT dataCleared();
	resetMinMax();
}

void DataMonitorModel::setXdata(const QVector<double> &newXdata)
{
	xdata.erase(xdata.begin(), xdata.end());
	xdata.append(newXdata);
	Q_EMIT dataCleared();
}

void DataMonitorModel::setMinValue(double newMinValue)
{
	m_minValue = newMinValue;
	Q_EMIT minValueUpdated(m_minValue);
}

void DataMonitorModel::setMaxValue(double newMaxValue)
{
	m_maxValue = newMaxValue;
	Q_EMIT maxValueUpdated(m_maxValue);
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

double DataMonitorModel::minValue() const { return m_minValue; }

double DataMonitorModel::maxValue() const { return m_maxValue; }

void DataMonitorModel::resetMinMax()
{
	for(double val : qAsConst(ydata)) {
		checkMinMaxUpdate(val);
	}
}

QString DataMonitorModel::getShortName() const { return shortName; }

void DataMonitorModel::setShortName(const QString &newShortName) { shortName = newShortName; }

QString DataMonitorModel::getDeviceName() const { return deviceName; }

void DataMonitorModel::setDeviceName(const QString &newDeviceName) { deviceName = newDeviceName; }

void DataMonitorModel::addValue(double time, double value)
{
	// make sure the total amount of data won't be more than what is set in preferences
	if(xdata.length() >= m_dataSize) {
		xdata.pop_front();
		ydata.pop_front();
	}

	if(ydata.isEmpty()) {
		setMinValue(value);
		setMaxValue(value);
	}

	// Apply scaling, offset, and unit of measurement scale
	double adjustedValue = (value + m_offset) * m_scale;

	xdata.push_back(time);
	ydata.push_back(adjustedValue);
	checkMinMaxUpdate(adjustedValue);

	Q_EMIT valueUpdated(time, adjustedValue);
}

double DataMonitorModel::defaultScale() const { return m_defaultScale; }

double DataMonitorModel::offset() const { return m_offset; }

void DataMonitorModel::setOffset(double newOffset)
{
	double oldOffset = m_offset;
	m_offset = newOffset;
	for(int i = 0; i < ydata.size(); ++i) {
		ydata[i] = ((ydata[i] / m_scale - oldOffset) + newOffset) * m_scale;
	}
	Q_EMIT dataCleared();
}

double DataMonitorModel::scale() const { return m_scale; }

void DataMonitorModel::setScale(double newScale)
{
	double oldScale = m_scale;
	m_scale = newScale;
	for(int i = 0; i < ydata.size(); ++i) {
		ydata[i] = ydata[i] / oldScale * newScale;
	}
	Q_EMIT dataCleared();
}

#include "moc_datamonitormodel.cpp"
