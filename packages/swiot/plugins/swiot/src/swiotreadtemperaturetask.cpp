/*
 * Copyright (c) 2023 Analog Devices Inc.
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

#include "swiotreadtemperaturetask.h"

#include "swiot_logging_categories.h"

#include <component/device.h>
#include <component/channel.h>
#include <component/attribute.h>
#include <component/attributereader.h>

#include <qcorotask.h>
#include <utility>

#define DEVICE_NAME "adt75"
#define CHANNEL_NAME "temp"

using namespace scopy::swiot;

SwiotReadTemperatureTask::SwiotReadTemperatureTask(QString uri, QObject *parent)
	: QObject(parent)
	, m_uri(std::move(uri))
	, m_channel(nullptr)
	, m_scale(0.0)
	, m_offset(0.0)
	, m_initialized(false)
{
	m_context = component::Controller::context(m_uri);
	component::Device *device = m_context
		? m_context->findChild<component::Device *>(DEVICE_NAME, Qt::FindDirectChildrenOnly)
		: nullptr;
	if(!device) {
		qDebug(CAT_SWIOT) << "Error, could not find" << DEVICE_NAME << ". Temperature not available.";
		return;
	}
	m_channel = device->findChild<component::Channel *>(CHANNEL_NAME, Qt::FindDirectChildrenOnly);
	if(!m_channel) {
		qDebug(CAT_SWIOT) << "Error, could not find channel" << CHANNEL_NAME << "from device" << DEVICE_NAME
				  << ". Temperature not available.";
	}
}

SwiotReadTemperatureTask::~SwiotReadTemperatureTask() { m_context = {}; }

QCoro::Task<void> SwiotReadTemperatureTask::initScaleOffset()
{
	component::Attribute *scaleAttr =
		m_channel->findChild<component::Attribute *>("scale", Qt::FindDirectChildrenOnly);
	component::Attribute *offsetAttr =
		m_channel->findChild<component::Attribute *>("offset", Qt::FindDirectChildrenOnly);

	if(scaleAttr && scaleAttr->readCapability()) {
		auto r = co_await scaleAttr->readCapability()->readAsync();
		bool ok = false;
		double v = scaleAttr->cachedValue().toDouble(&ok);
		if(r && ok) {
			m_scale = v;
		}
	}
	if(offsetAttr && offsetAttr->readCapability()) {
		auto r = co_await offsetAttr->readCapability()->readAsync();
		bool ok = false;
		double v = offsetAttr->cachedValue().toDouble(&ok);
		if(r && ok) {
			m_offset = v;
		}
	}
	m_initialized = true;
}

QCoro::Task<void> SwiotReadTemperatureTask::readTemperature()
{
	if(!m_channel) {
		co_return;
	}
	if(!m_initialized) {
		co_await initScaleOffset();
	}
	component::Attribute *rawAttr = m_channel->findChild<component::Attribute *>("raw", Qt::FindDirectChildrenOnly);
	if(!rawAttr || !rawAttr->readCapability()) {
		co_return;
	}
	auto r = co_await rawAttr->readCapability()->readAsync();
	if(!r) {
		qDebug(CAT_SWIOT) << "Error, could not read \"raw\" attribute from" << DEVICE_NAME
				  << ". Temperature not available.";
		co_return;
	}
	bool ok = false;
	double raw = rawAttr->cachedValue().toDouble(&ok);
	if(ok) {
		double temperature = (raw + m_offset) * m_scale / 1000;
		qDebug(CAT_SWIOT) << "Read temperature value of" << temperature;
		Q_EMIT newTemperature(temperature);
	}
}

#include "moc_swiotreadtemperaturetask.cpp"
