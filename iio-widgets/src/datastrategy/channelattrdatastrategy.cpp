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

#include "datastrategy/channelattrdatastrategy.h"
#include <QFile>
#define BUFFER_SIZE 16384

Q_LOGGING_CATEGORY(CAT_IIO_DATA_STRATEGY, "AttrDataStrategy")
using namespace scopy;

ChannelAttrDataStrategy::ChannelAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent)
	: QWidget(parent)
{
	m_recipe = recipe;
}

QString ChannelAttrDataStrategy::data() { return m_data; }

QString ChannelAttrDataStrategy::optionalData() { return m_optionalData; }

void ChannelAttrDataStrategy::save(QString data)
{
	if(m_recipe.channel == nullptr || m_recipe.data == "") {
		qWarning(CAT_IIO_DATA_STRATEGY) << "Invalid arguments, cannot write any data";
		return;
	}

	Q_EMIT aboutToWrite(m_data, data);
	ssize_t res = iio_channel_attr_write(m_recipe.channel, m_recipe.data.toStdString().c_str(),
					     data.toStdString().c_str());
	if(res < 0) {
		qWarning(CAT_IIO_DATA_STRATEGY) << "Cannot write" << data << "to" << m_recipe.data;
	}

	Q_EMIT emitStatus(QDateTime::currentDateTime(), m_data, data, (int)(res), false);
	requestData(); // readback
}

void ChannelAttrDataStrategy::requestData()
{
	if(m_recipe.channel == nullptr || m_recipe.data.isEmpty()) {
		qWarning(CAT_IIO_DATA_STRATEGY) << "Invalid arguments, cannot read any data";
		return;
	}

	char options[BUFFER_SIZE] = {0}, currentValue[BUFFER_SIZE] = {0};

	ssize_t currentValueResult =
		iio_channel_attr_read(m_recipe.channel, m_recipe.data.toStdString().c_str(), currentValue, BUFFER_SIZE);
	if(currentValueResult < 0) {
		qWarning(CAT_IIO_DATA_STRATEGY)
			<< "Could not read" << m_recipe.data << "error code:" << currentValueResult;
	}

	if(m_recipe.iioDataOptions != "") {
		ssize_t optionsResult = iio_channel_attr_read(
			m_recipe.channel, m_recipe.iioDataOptions.toStdString().c_str(), options, BUFFER_SIZE);
		if(optionsResult < 0) {
			qWarning(CAT_IIO_DATA_STRATEGY)
				<< "Could not read" << m_recipe.data << "error code:" << optionsResult;
		}
		Q_EMIT emitStatus(QDateTime::currentDateTime(), m_optionalData, options, (int)(currentValueResult),
				  true);
	}

	if(m_recipe.constDataOptions != "") {
		if(m_recipe.constDataOptions.size() >= BUFFER_SIZE) {
			qWarning(CAT_IIO_DATA_STRATEGY) << "The data from constDataOptions exceeds the buffer size. "
							   "Consider updating one of them.";
		}

		strncpy(options, m_recipe.constDataOptions.toStdString().c_str(), m_recipe.constDataOptions.size());
		options[m_recipe.constDataOptions.size()] = '\0'; // safety measures
	}

	// the members that contain data need to be refreshed before the signals are emitted
	QString oldData = m_data;
	m_data = currentValue;
	m_optionalData = options;
	Q_EMIT emitStatus(QDateTime::currentDateTime(), currentValue, m_data, (int)(currentValueResult), true);
	Q_EMIT sendData(m_data, m_optionalData);
}

#include "moc_channelattrdatastrategy.cpp"
