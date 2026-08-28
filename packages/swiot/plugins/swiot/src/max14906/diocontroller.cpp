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

#include "max14906/diocontroller.h"
#include "max14906/max14906.h"
#include "swiot_logging_categories.h"

#include <component/device.h>
#include <component/channel.h>

using namespace scopy::swiot;
using namespace scopy;

DioController::DioController(component::Device *device, QString deviceName)
	: m_deviceName(deviceName)
	, m_device(device)
{
	if(m_device) {
		m_channels = m_device->findChildren<component::Channel *>(Qt::FindDirectChildrenOnly);
	}
}

DioController::~DioController() {}

int DioController::getChannelCount() { return m_channels.size(); }

component::Channel *DioController::getChannel(unsigned int index)
{
	if(index >= (unsigned int)m_channels.size()) {
		qCritical(CAT_SWIOT_MAX14906) << "Error when selecting channel with index" << index;
		return nullptr;
	}
	return m_channels[index];
}

QString DioController::getChannelName(unsigned int index)
{
	component::Channel *channel = getChannel(index);
	if(channel == nullptr) {
		return "";
	}
	return channel->id();
}

QString DioController::getChannelType(unsigned int index)
{
	component::Channel *channel = getChannel(index);
	if(channel == nullptr) {
		return "";
	}
	return channel->isOutput() ? "OUTPUT" : "INPUT";
}

component::Device *DioController::getDevice() const { return DioController::m_device; }

#include "moc_diocontroller.cpp"
