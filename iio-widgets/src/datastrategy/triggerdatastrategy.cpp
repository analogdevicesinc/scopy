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

#include "datastrategy/triggerdatastrategy.h"
#include <utility>
#include <QFile>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_TRIGGER_DATA_STRATEGY, "TriggerDataStrategy")

TriggerDataStrategy::TriggerDataStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent)
	: QWidget(parent)
{
	m_recipe = recipe;
}

QString TriggerDataStrategy::data() { return m_data; }

QString TriggerDataStrategy::optionalData() { return m_optionalData; }

void TriggerDataStrategy::save(QString data)
{
	if(m_recipe.context == nullptr || m_recipe.device == nullptr) {
		qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Invalid arguments, cannot write any data";
		return;
	}

	struct iio_device *trigger = iio_context_find_device(m_recipe.context, data.toStdString().c_str());
	if(trigger == nullptr) {
		qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Invalid arguments, no trigger with name" << data << "was found";
	}

	Q_EMIT aboutToWrite(m_data, data);
	int res;
	if(data == "None") {
		res = iio_device_set_trigger(m_recipe.device, nullptr);

		if(res < 0) {
			qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Cannot clear trigger";
		}
	} else {
		res = iio_device_set_trigger(m_recipe.device, trigger);

		if(res < 0) {
			qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Cannot set trigger" << data;
		}
	}

	Q_EMIT emitStatus(QDateTime::currentDateTime(), m_data, data, (int)(res), false);
	requestData();
}

void TriggerDataStrategy::requestData()
{
	QString currentTriggerName, triggerOptions = "None ";

	if(m_recipe.context == nullptr || m_recipe.device == nullptr) {
		qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Invalid arguments, cannot read any data";
		return;
	}

	const struct iio_device *currentTrigger;
	ssize_t res = iio_device_get_trigger(m_recipe.device, &currentTrigger);
	if(res < 0) {
		qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Cannot read trigger";
		currentTrigger = nullptr;
		currentTriggerName = "None";
	}

	if(currentTrigger != nullptr) {
		currentTriggerName = iio_device_get_name(currentTrigger);
	}

	unsigned int deviceCount = iio_context_get_devices_count(m_recipe.context);
	for(int i = 0; i < deviceCount; ++i) {
		struct iio_device *dev = iio_context_get_device(m_recipe.context, i);
		if(dev == nullptr) {
			qDebug(CAT_TRIGGER_DATA_STRATEGY) << "No device with index" << i << "was found.";
			continue;
		}
		bool isTrigger = iio_device_is_trigger(dev);
		if(isTrigger) {
			QString name = iio_device_get_name(dev);
			if(!name.isEmpty()) {
				triggerOptions += name + " ";
			}
		}
	}

	Q_EMIT emitStatus(QDateTime::currentDateTime(), m_data, currentTriggerName, res, true);
	m_data = currentTriggerName;
	m_optionalData = triggerOptions;
	Q_EMIT sendData(m_data, m_optionalData);
}

#include "moc_triggerdatastrategy.cpp"
