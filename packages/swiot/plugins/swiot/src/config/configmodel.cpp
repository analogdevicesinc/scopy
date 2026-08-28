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

#include "configmodel.h"
#include "swiot_logging_categories.h"

#include <component/device.h>
#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/attributewriter.h>

#include <qcorotask.h>

using namespace scopy::swiot;
using namespace scopy;

ConfigModel::ConfigModel(component::Device *device, int channelId)
	: m_device(device)
	, m_channelId(channelId)
{
	QString attributePrefix = "ch" + QString::number(m_channelId);

	m_enableAttribute = attributePrefix + "_enable";
	m_functionAttribute = attributePrefix + "_function";
	m_functionAvailableAttribute = attributePrefix + "_function_available";
	m_deviceAttribute = attributePrefix + "_device";
	m_deviceAvailableAttribute = attributePrefix + "_device_available";
}

ConfigModel::~ConfigModel() {}

component::Attribute *ConfigModel::getAttr(const QString &name)
{
	if(!m_device) {
		return nullptr;
	}
	return m_device->findChild<component::Attribute *>(name, Qt::FindDirectChildrenOnly);
}

QCoro::Task<void> ConfigModel::readEnabled()
{
	component::Attribute *attr = getAttr(m_enableAttribute);
	if(!attr || !attr->readCapability()) {
		qCritical(CAT_SWIOT_CONFIG) << "Error: could not read attribute \"enable\" on channel" << m_channelId;
		co_return;
	}
	auto r = co_await attr->readCapability()->readAsync();
	if(!r) {
		qCritical(CAT_SWIOT_CONFIG) << "Error: could not read attribute \"enable\" on channel" << m_channelId;
		co_return;
	}
	bool ok = false;
	bool enabled = attr->cachedValue().toInt(&ok);
	if(ok) {
		Q_EMIT readConfigChannelEnabled(enabled);
	}
}

QCoro::Task<void> ConfigModel::writeEnabled(const QString &enabled)
{
	component::Attribute *attr = getAttr(m_enableAttribute);
	if(!attr || !attr->writeCapability()) {
		co_return;
	}
	auto r = co_await attr->writeCapability()->writeAsync(enabled);
	if(!r) {
		qCritical(CAT_SWIOT_CONFIG)
			<< "Error: could not write attribute \"enable\", (" << enabled << ") on channel" << m_channelId;
	} else {
		Q_EMIT configChannelEnabled();
	}
}

QCoro::Task<void> ConfigModel::readDevice()
{
	component::Attribute *attr = getAttr(m_deviceAttribute);
	if(!attr || !attr->readCapability()) {
		co_return;
	}
	auto r = co_await attr->readCapability()->readAsync();
	if(!r) {
		qDebug(CAT_SWIOT_CONFIG) << "Can't read value from device on channel" << m_channelId;
		co_return;
	}
	Q_EMIT readConfigChannelDevice(attr->cachedValue());
}

QCoro::Task<void> ConfigModel::writeDevice(const QString &device)
{
	component::Attribute *attr = getAttr(m_deviceAttribute);
	if(!attr || !attr->writeCapability()) {
		co_return;
	}
	auto r = co_await attr->writeCapability()->writeAsync(device);
	if(!r) {
		qCritical(CAT_SWIOT_CONFIG)
			<< "Error: could not write attribute \"device\", (" << device << ") on channel" << m_channelId;
	} else {
		Q_EMIT configChannelDevice();
	}
}

QCoro::Task<void> ConfigModel::readFunction()
{
	component::Attribute *attr = getAttr(m_functionAttribute);
	if(!attr || !attr->readCapability()) {
		qCritical(CAT_SWIOT_CONFIG) << "Error: could not read attribute \"function\" on channel" << m_channelId;
		co_return;
	}
	auto r = co_await attr->readCapability()->readAsync();
	if(!r) {
		qCritical(CAT_SWIOT_CONFIG) << "Error: could not read attribute \"function\" on channel" << m_channelId;
		co_return;
	}
	Q_EMIT readConfigChannelFunction(attr->cachedValue());
}

QCoro::Task<void> ConfigModel::writeFunction(const QString &function)
{
	component::Attribute *attr = getAttr(m_functionAttribute);
	if(!attr || !attr->writeCapability()) {
		co_return;
	}
	auto r = co_await attr->writeCapability()->writeAsync(function);
	if(!r) {
		qCritical(CAT_SWIOT_CONFIG) << "Error: could not write attribute \"function\", (" << function
					    << ") on channel" << m_channelId;
	} else {
		Q_EMIT configChannelFunction();
	}
}

QCoro::Task<void> ConfigModel::readDeviceAvailable()
{
	component::Attribute *availAttr = getAttr(m_deviceAvailableAttribute);
	if(availAttr && availAttr->readCapability()) {
		auto r = co_await availAttr->readCapability()->readAsync();
		if(r) {
			Q_EMIT readConfigChannelDeviceAvailable(availAttr->cachedValue().split(" "));
			co_return;
		}
	}
	component::Attribute *attr = getAttr(m_deviceAttribute);
	if(attr && attr->hasOptions()) {
		Q_EMIT readConfigChannelDeviceAvailable(attr->options());
		co_return;
	}
	qCritical(CAT_SWIOT_CONFIG) << "Error: could not read attribute \"device available\" on channel" << m_channelId;
}

QCoro::Task<void> ConfigModel::readFunctionAvailable()
{
	component::Attribute *availAttr = getAttr(m_functionAvailableAttribute);
	if(availAttr && availAttr->readCapability()) {
		auto r = co_await availAttr->readCapability()->readAsync();
		if(r) {
			Q_EMIT readConfigChannelFunctionAvailable(availAttr->cachedValue().split(" "));
			co_return;
		}
	}
	component::Attribute *attr = getAttr(m_functionAttribute);
	if(attr && attr->hasOptions()) {
		Q_EMIT readConfigChannelFunctionAvailable(attr->options());
		co_return;
	}
	qCritical(CAT_SWIOT_CONFIG) << "Error: could not read attribute \"function available\" on channel"
				    << m_channelId;
}
