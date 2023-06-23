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
#include <iio.h>
#include <QMap>
#include "src/swiot_logging_categories.h"

using namespace scopy::swiot;

ConfigModel::ConfigModel(struct iio_device* device, int channelId) :
	m_device(device),
	m_channelId(channelId)
{
	std::string attributePrefix = "ch" + std::to_string(m_channelId);

	m_enableAttribute = attributePrefix + "_enable";
	m_functionAttribute = attributePrefix + "_function";
	m_functionAvailableAttribute = attributePrefix + "_function_available";
	m_deviceAttribute = attributePrefix + "_device";
	m_deviceAvailableAttribute = attributePrefix + "_device_available";
}

ConfigModel::~ConfigModel()
{}

QString ConfigModel::readEnabled() {
	char buffer[16] = {0};
	ssize_t res = iio_device_attr_read(m_device, m_enableAttribute.c_str(), buffer, 15);
	if (res < 0) {
		qCritical(CAT_SWIOT_CONFIG) << "Error: could not read attribute \"enable\" on channel" << m_channelId << "error id ->" << res;
	} else {
		qDebug(CAT_SWIOT_CONFIG) << "Read value \"" << buffer << "\" from enable on channel" << m_channelId;
		return {buffer};
	}

	return "0";
}

void ConfigModel::writeEnabled(const QString& enabled) {
	ssize_t res = iio_device_attr_write(m_device, m_enableAttribute.c_str(), enabled.toStdString().c_str());
	if (res < 0) {
		qCritical(CAT_SWIOT_CONFIG) << "Error: could not write attribute \"enable\", (" << enabled << ") on channel" << m_channelId << "error id ->" << res;
	} else {
		qDebug(CAT_SWIOT_CONFIG) << "Wrote value " << enabled << "in enable on channel" << m_channelId;
	}
}

QString ConfigModel::readDevice() {
	char buffer[256] = {0};
	ssize_t res = iio_device_attr_read(m_device, m_deviceAttribute.c_str(), buffer, 255);
	if (res < 0) {
		qCritical(CAT_SWIOT_CONFIG) << "Error: could not read attribute \"device\" on channel" << m_channelId << "error id ->" << res;
	} else {
		qDebug(CAT_SWIOT_CONFIG) << "Read value \"" << buffer << "\" from device on channel" << m_channelId;
		return {buffer};
	}

	return "0";
}

void ConfigModel::writeDevice(const QString& device) {
	ssize_t res = iio_device_attr_write(m_device, m_deviceAttribute.c_str(), device.toStdString().c_str());
	if (res < 0) {
		qCritical(CAT_SWIOT_CONFIG) << "Error: could not write attribute \"device\", (" << device << ")  on channel" << m_channelId << "error id ->" << res;
	} else {
		qDebug(CAT_SWIOT_CONFIG) << "Wrote value " << device << "in device on channel" << m_channelId;
	}
}

QString ConfigModel::readFunction() {
	char buffer[256] = {0};
	ssize_t res = iio_device_attr_read(m_device, m_functionAttribute.c_str(), buffer, 255);
	if (res < 0) {
		qCritical(CAT_SWIOT_CONFIG) << "Error: could not read attribute \"function\" on channel" << m_channelId << "error id ->" << res;
	} else {
		qDebug(CAT_SWIOT_CONFIG) << "Read value \"" << buffer << "\" from function on channel" << m_channelId;
		return {buffer};
	}

	return "0";
}

void ConfigModel::writeFunction(const QString& function) {
	ssize_t res = iio_device_attr_write(m_device, m_functionAttribute.c_str(), function.toStdString().c_str());
	if (res < 0) {
		qCritical(CAT_SWIOT_CONFIG) << "Error: could not write attribute \"function\", (" << function << ")  on channel" << m_channelId << "error id ->" << res;
	} else {
		qDebug(CAT_SWIOT_CONFIG) << "Wrote value " << function << "in function on channel" << m_channelId;
	}
}

QStringList ConfigModel::readDeviceAvailable() {
	char buffer[512] = {0};
	ssize_t res = iio_device_attr_read(m_device, m_deviceAvailableAttribute.c_str(), buffer, 511);
	if (res < 0) {
		qCritical(CAT_SWIOT_CONFIG) << "Error: could not read attribute \"device available\" on channel" << m_channelId << "error id ->" << res;
		return {};
	} else {
		qDebug(CAT_SWIOT_CONFIG) << "Read value \"" << buffer << "\" from device available on channel" << m_channelId;
	}

	QStringList result = QString(buffer).split(" ");
	return result;
}

QStringList ConfigModel::readFunctionAvailable() {
	char buffer[512] = {0};
	ssize_t res = iio_device_attr_read(m_device, m_functionAvailableAttribute.c_str(), buffer, 511);
	if (res < 0) {
		qCritical(CAT_SWIOT_CONFIG) << "Error: could not read attribute \"function available\" on channel" << m_channelId << "error id ->" << res;
		return {};
	} else {
		qDebug(CAT_SWIOT_CONFIG) << "Read value \"" << buffer << "\" from function available on channel" << m_channelId;
	}

	QStringList result = QString(buffer).split(" ");
	return result;
}



