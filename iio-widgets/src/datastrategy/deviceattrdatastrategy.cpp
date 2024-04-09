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
 */

#include "datastrategy/deviceattrdatastrategy.h"
#define BUFFER_SIZE 16384

Q_LOGGING_CATEGORY(CAT_DEVICE_DATA_STRATEGY, "DeviceDataStrategy")
using namespace scopy;

DeviceAttrDataStrategy::DeviceAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent)
	: QWidget(parent)
{
	m_recipe = recipe;
}

QString DeviceAttrDataStrategy::data() { return m_data; }

QString DeviceAttrDataStrategy::optionalData() { return m_optionalData; }

void DeviceAttrDataStrategy::save(QString data)
{
	if(m_recipe.device == nullptr || m_recipe.data == "") {
		qWarning(CAT_DEVICE_DATA_STRATEGY) << "Invalid arguments, cannot write any data";
		return;
	}

	Q_EMIT aboutToWrite(m_data, data);
	ssize_t res =
		iio_device_attr_write(m_recipe.device, m_recipe.data.toStdString().c_str(), data.toStdString().c_str());
	if(res < 0) {
		qWarning(CAT_DEVICE_DATA_STRATEGY) << "Cannot write" << data << "to" << m_recipe.data;
	}

	Q_EMIT emitStatus((int)(res));
	requestData();
}

void DeviceAttrDataStrategy::requestData()
{
	if(m_recipe.device == nullptr || m_recipe.data.isEmpty()) {
		qWarning(CAT_DEVICE_DATA_STRATEGY) << "Invalid arguments, cannot read any data";
		return;
	}

	char options[BUFFER_SIZE] = {0}, currentValue[BUFFER_SIZE] = {0};
	ssize_t currentValueResult =
		iio_device_attr_read(m_recipe.device, m_recipe.data.toStdString().c_str(), currentValue, BUFFER_SIZE);
	if(currentValueResult < 0) {
		qWarning(CAT_DEVICE_DATA_STRATEGY)
			<< "Could not read" << m_recipe.data << "error code:" << currentValueResult;
	}

	if(m_recipe.iioDataOptions != "") {
		ssize_t optionsResult = iio_device_attr_read(
			m_recipe.device, m_recipe.iioDataOptions.toStdString().c_str(), options, BUFFER_SIZE);
		if(optionsResult < 0) {
			qWarning(CAT_DEVICE_DATA_STRATEGY)
				<< "Could not read" << m_recipe.data << "error code:" << optionsResult;
		}
	}

	if(m_recipe.constDataOptions != "") {
		if(m_recipe.constDataOptions.size() >= BUFFER_SIZE) {
			qWarning(CAT_DEVICE_DATA_STRATEGY) << "The data from constDataOptions exceeds the buffer size. "
							      "Consider updating one of them.";
		}

		strncpy(options, m_recipe.constDataOptions.toStdString().c_str(), m_recipe.constDataOptions.size());
		options[m_recipe.constDataOptions.size()] = '\0'; // safety measures
	}

	m_data = currentValue;
	m_optionalData = options;
	Q_EMIT sendData(m_data, m_optionalData);
}

#include "moc_deviceattrdatastrategy.cpp"
