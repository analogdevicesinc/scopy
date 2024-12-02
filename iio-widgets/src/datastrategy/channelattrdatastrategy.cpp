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

void ChannelAttrDataStrategy::save(QString data)
{
	if(m_recipe.channel == nullptr || m_recipe.data == "") {
		qWarning(CAT_IIO_DATA_STRATEGY) << "Invalid arguments, cannot write any data";
		return;
	}

	ssize_t res = iio_channel_attr_write(m_recipe.channel, m_recipe.data.toStdString().c_str(),
					     data.toStdString().c_str());
	if(res < 0) {
		qWarning(CAT_IIO_DATA_STRATEGY) << "Cannot write" << data << "to" << m_recipe.data;
	}

	Q_EMIT emitStatus((int)(res));
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

	if(m_recipe.dataOptions != "") {
		ssize_t optionsResult = iio_channel_attr_read(
			m_recipe.channel, m_recipe.dataOptions.toStdString().c_str(), options, BUFFER_SIZE);
		if(optionsResult < 0) {
			qWarning(CAT_IIO_DATA_STRATEGY)
				<< "Could not read" << m_recipe.data << "error code:" << optionsResult;
		}
	}

	Q_EMIT sendData(QString(currentValue), QString(options));
}

#include "moc_channelattrdatastrategy.cpp"
