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

#include "datamonitor/readabledatamonitormodel.hpp"

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

#include "moc_readabledatamonitormodel.cpp"
