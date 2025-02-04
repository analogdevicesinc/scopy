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
#include <iioutil/iiocpp/iiodevice.h>
#include <iioutil/iiocpp/iioresult.h>
#include <iioutil/iiocpp/iiocontext.h>
#include <QFile>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_TRIGGER_DATA_STRATEGY, "TriggerDataStrategy")

TriggerDataStrategy::TriggerDataStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent)
	: QObject(parent)
{
	m_recipe = recipe;
}

TriggerDataStrategy::~TriggerDataStrategy() {}

QString TriggerDataStrategy::data() { return m_data; }

QString TriggerDataStrategy::optionalData() { return m_optionalData; }

void TriggerDataStrategy::writeAsync(QString data)
{
	int res = write(data);
	Q_EMIT emitStatus(QDateTime::currentDateTime(), m_data, data, (int)(res), false);
	readAsync();
}

void TriggerDataStrategy::readAsync()
{
	read();

	Q_EMIT emitStatus(QDateTime::currentDateTime(), m_previousData, m_data, m_returnCode, true);
	Q_EMIT sendData(m_data, m_optionalData);
}

int TriggerDataStrategy::write(QString data)
{
	if(m_recipe.context == nullptr || m_recipe.device == nullptr) {
		qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Invalid arguments, cannot write any data";
		return -EINVAL;
	}

	IIOResult<iio_device *> triggerRes = IIOContext::find_device(m_recipe.context, data.toStdString().c_str());
	if(!triggerRes.ok()) {
		qWarning(CAT_TRIGGER_DATA_STRATEGY)
			<< "Invalid arguments, no trigger with name" << data << "was found" << triggerRes.error();
	}

	Q_EMIT aboutToWrite(m_data, data);
	int res;
	if(data == "None") {
		res = IIODevice::set_trigger(m_recipe.device, nullptr);

		if(res < 0) {
			qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Cannot clear trigger";
		}
	} else {
		res = IIODevice::set_trigger(m_recipe.device, triggerRes.data());

		if(res < 0) {
			qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Cannot set trigger" << data;
		}
	}

	return res;
}

QPair<QString, QString> TriggerDataStrategy::read()
{
	QString currentTriggerName, triggerOptions = "None ";

	if(m_recipe.context == nullptr || m_recipe.device == nullptr) {
		qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Invalid arguments, cannot read any data";
		return {};
	}

	const iio_device *currentTrigger;
	IIOResult<const iio_device *> triggerRes = IIODevice::get_trigger(m_recipe.device);
	if(!triggerRes.ok()) {
		qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Cannot read trigger";
		currentTrigger = nullptr;
		currentTriggerName = "None";
	} else {
		currentTrigger = triggerRes.data();
		currentTriggerName = IIODevice::get_name(currentTrigger);
	}

	unsigned int deviceCount = IIOContext::get_devices_count(m_recipe.context);
	for(int i = 0; i < deviceCount; ++i) {
		IIOResult<iio_device *> devRes = IIOContext::get_device(m_recipe.context, i);
		if(!devRes.ok()) {
			qWarning(CAT_TRIGGER_DATA_STRATEGY) << "Cannot get device" << i << "error:" << devRes.error();
			continue;
		}
		iio_device *dev = devRes.data();

		bool isTrigger = IIODevice::is_trigger(dev);
		if(isTrigger) {
			QString name = IIODevice::get_name(dev);
			if(!name.isEmpty()) {
				triggerOptions += name + " ";
			}
		}
	}

	m_previousData = m_data;
	m_data = currentTriggerName;
	m_optionalData = triggerOptions;

	return {m_data, m_previousData};
}

#include "moc_triggerdatastrategy.cpp"
