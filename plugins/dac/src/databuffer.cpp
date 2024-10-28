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

#include "databuffer.h"

#include <QWidget>

using namespace scopy;
using namespace scopy::dac;

DataBuffer::DataBuffer(DataGuiStrategyInterface *guids, DataBufferStrategyInterface *ds, QWidget *parent)
	: QObject(parent)
	, m_dataStrategy(ds)
	, m_guiStrategy(guids)
{
	m_parent = parent;
	if(m_guiStrategy) {
		connect(dynamic_cast<QObject *>(m_guiStrategy), SIGNAL(recipeUpdated(DataBufferRecipe)),
			dynamic_cast<QObject *>(m_dataStrategy), SLOT(recipeUpdated(DataBufferRecipe)));
	}
	connect(dynamic_cast<QObject *>(m_dataStrategy), SIGNAL(loadFinished()), this, SIGNAL(loadFinished()));
	connect(dynamic_cast<QObject *>(m_dataStrategy), SIGNAL(loadFailed()), this, SIGNAL(loadFailed()));
	connect(dynamic_cast<QObject *>(m_dataStrategy), SIGNAL(dataUpdated()), this, SIGNAL(dataUpdated()));
}

DataBuffer::~DataBuffer() {}

DataBufferStrategyInterface *DataBuffer::getDataBufferStrategy() { return m_dataStrategy; }

DataGuiStrategyInterface *DataBuffer::getDataGuiStrategyInterface() { return m_guiStrategy; }

QWidget *DataBuffer::getParent() { return m_parent; }

void DataBuffer::loadData() { m_dataStrategy->loadData(); }
