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

ChannelAttrDataStrategy::ChannelAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent)
	: QObject(parent)
{
	m_recipe = recipe;
	m_returnCode = 0;
}

ChannelAttrDataStrategy::~ChannelAttrDataStrategy() {}

QString ChannelAttrDataStrategy::data() { return m_data; }

QString ChannelAttrDataStrategy::optionalData() { return m_optionalData; }

void ChannelAttrDataStrategy::writeAsync(QString data)
{
	int retCode = write(data);

	Q_EMIT emitStatus(QDateTime::currentDateTime(), m_data, data, (int)(retCode), false);
	readAsync(); // readback
}

void ChannelAttrDataStrategy::readAsync()
{
	QPair<QString, QString> values = read();

	Q_EMIT emitStatus(QDateTime::currentDateTime(), values.first, m_data, (int)(m_returnCode), true);
	Q_EMIT sendData(m_data, m_optionalData);
}

int ChannelAttrDataStrategy::write(QString data)
{
	if(m_recipe.channel == nullptr || m_recipe.data == "") {
		qWarning(CAT_IIO_DATA_STRATEGY) << "Invalid arguments, cannot write any data";
		return -EINVAL;
	}

	Q_EMIT aboutToWrite(m_data, data);
	ssize_t res = iio_channel_attr_write(m_recipe.channel, m_recipe.data.toStdString().c_str(),
					     data.toStdString().c_str());
	if(res < 0) {
		qWarning(CAT_IIO_DATA_STRATEGY) << "Cannot write" << data << "to" << m_recipe.data;
	}

	return res;
}

QPair<QString, QString> ChannelAttrDataStrategy::read()
{
	if(m_recipe.channel == nullptr || m_recipe.data.isEmpty()) {
		qWarning(CAT_IIO_DATA_STRATEGY) << "Invalid arguments, cannot read any data";
		return {};
	}

	char options[BUFFER_SIZE] = {0}, currentValue[BUFFER_SIZE] = {0};

	m_returnCode =
		iio_channel_attr_read(m_recipe.channel, m_recipe.data.toStdString().c_str(), currentValue, BUFFER_SIZE);
	if(m_returnCode < 0) {
		qWarning(CAT_IIO_DATA_STRATEGY) << "Could not read" << m_recipe.data << "error code:" << m_returnCode;
	}

	if(m_recipe.iioDataOptions != "") {
		ssize_t optionsResult = iio_channel_attr_read(
			m_recipe.channel, m_recipe.iioDataOptions.toStdString().c_str(), options, BUFFER_SIZE);
		if(optionsResult < 0) {
			qWarning(CAT_IIO_DATA_STRATEGY)
				<< "Could not read" << m_recipe.data << "error code:" << optionsResult;
		}
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

	return {currentValue, options};
}

#include "moc_channelattrdatastrategy.cpp"
