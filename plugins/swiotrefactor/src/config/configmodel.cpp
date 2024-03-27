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

#include <iio.h>
#include <iioutil/iiocommand/iiodeviceattributeread.h>
#include <iioutil/iiocommand/iiodeviceattributewrite.h>

using namespace scopy::swiotrefactor;

ConfigModel::ConfigModel(struct iio_device *device, int channelId, CommandQueue *commandQueue)
	: m_device(device)
	, m_channelId(channelId)
	, m_commandQueue(commandQueue)
{
	QString attributePrefix = "ch" + QString::number(m_channelId);

	m_enableAttribute = attributePrefix + "_enable";
	m_functionAttribute = attributePrefix + "_function";
	m_functionAvailableAttribute = attributePrefix + "_function_available";
	m_deviceAttribute = attributePrefix + "_device";
	m_deviceAvailableAttribute = attributePrefix + "_device_available";
}

ConfigModel::~ConfigModel() {}

void ConfigModel::readEnabled()
{
	Command *enabledChnCmd = new IioDeviceAttributeRead(m_device, m_enableAttribute.toStdString().c_str(), nullptr);
	connect(
		enabledChnCmd, &scopy::Command::finished, this,
		[=, this](scopy::Command *cmd) {
			IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
			if(!tcmd) {
				return;
			}

			if(tcmd->getReturnCode() >= 0) {
				char *result = tcmd->getResult();
				bool ok = false;
				bool enabled = QString(result).toInt(&ok);
				if(!ok) {
					return;
				}
				Q_EMIT readConfigChannelEnabled(enabled);
			} else {
				qCritical(CAT_SWIOT_CONFIG) << "Error: could not read attribute \"enable\" on channel"
							    << m_channelId << "error id ->" << tcmd->getReturnCode();
			}
		},
		Qt::QueuedConnection);
	m_commandQueue->enqueue(enabledChnCmd);
}

void ConfigModel::writeEnabled(const QString &enabled)
{
	Command *enabledChnCmd = new IioDeviceAttributeWrite(m_device, m_enableAttribute.toStdString().c_str(),
							     enabled.toStdString().c_str(), nullptr);
	connect(
		enabledChnCmd, &scopy::Command::finished, this,
		[=, this](scopy::Command *cmd) {
			IioDeviceAttributeWrite *tcmd = dynamic_cast<IioDeviceAttributeWrite *>(cmd);
			if(!tcmd) {
				return;
			}
			if(tcmd->getReturnCode() < 0) {
				qCritical(CAT_SWIOT_CONFIG)
					<< "Error: could not write attribute \"enable\", (" << enabled << ") on channel"
					<< m_channelId << "error id ->" << tcmd->getReturnCode();
			} else {
				Q_EMIT configChannelEnabled();
			}
		},
		Qt::QueuedConnection);
	m_commandQueue->enqueue(enabledChnCmd);
}

void ConfigModel::readDevice()
{
	Command *deviceChnCmd = new IioDeviceAttributeRead(m_device, m_deviceAttribute.toStdString().c_str(), nullptr);
	connect(
		deviceChnCmd, &scopy::Command::finished, this,
		[=, this](scopy::Command *cmd) {
			IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
			if(!tcmd) {
				return;
			}

			if(tcmd->getReturnCode() >= 0) {
				char *result = tcmd->getResult();
				Q_EMIT readConfigChannelDevice(result);
			} else {
				qDebug(CAT_SWIOT_CONFIG) << "Can't read value from device on channel" << m_channelId
							 << "error id ->" << tcmd->getReturnCode();
			}
		},
		Qt::QueuedConnection);
	m_commandQueue->enqueue(deviceChnCmd);
}

void ConfigModel::writeDevice(const QString &device)
{
	Command *deviceChnCmd = new IioDeviceAttributeWrite(m_device, m_deviceAttribute.toStdString().c_str(),
							    device.toStdString().c_str(), nullptr);
	connect(
		deviceChnCmd, &scopy::Command::finished, this,
		[=, this](scopy::Command *cmd) {
			IioDeviceAttributeWrite *tcmd = dynamic_cast<IioDeviceAttributeWrite *>(cmd);
			if(!tcmd) {
				return;
			}
			if(tcmd->getReturnCode() < 0) {
				qCritical(CAT_SWIOT_CONFIG)
					<< "Error: could not write attribute \"device\", (" << device << ") on channel"
					<< m_channelId << "error id ->" << tcmd->getReturnCode();
			} else {
				Q_EMIT configChannelDevice();
			}
		},
		Qt::QueuedConnection);
	m_commandQueue->enqueue(deviceChnCmd);
}

void ConfigModel::readFunction()
{
	Command *functionChnCmd =
		new IioDeviceAttributeRead(m_device, m_functionAttribute.toStdString().c_str(), nullptr);
	connect(
		functionChnCmd, &scopy::Command::finished, this,
		[=, this](scopy::Command *cmd) {
			IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
			if(!tcmd) {
				return;
			}

			if(tcmd->getReturnCode() >= 0) {
				char *result = tcmd->getResult();
				Q_EMIT readConfigChannelFunction(result);
			} else {
				qCritical(CAT_SWIOT_CONFIG) << "Error: could not read attribute \"function\" on channel"
							    << m_channelId << "error id ->" << tcmd->getReturnCode();
			}
		},
		Qt::QueuedConnection);
	m_commandQueue->enqueue(functionChnCmd);
}

void ConfigModel::writeFunction(const QString &function)
{
	Command *functionChnCmd = new IioDeviceAttributeWrite(m_device, m_functionAttribute.toStdString().c_str(),
							      function.toStdString().c_str(), nullptr);
	connect(
		functionChnCmd, &scopy::Command::finished, this,
		[=, this](scopy::Command *cmd) {
			IioDeviceAttributeWrite *tcmd = dynamic_cast<IioDeviceAttributeWrite *>(cmd);
			if(!tcmd) {
				return;
			}
			if(tcmd->getReturnCode() < 0) {
				qCritical(CAT_SWIOT_CONFIG)
					<< "Error: could not write attribute \"function\", (" << function
					<< ") on channel" << m_channelId << "error id ->" << tcmd->getReturnCode();
			} else {
				Q_EMIT configChannelFunction();
			}
		},
		Qt::QueuedConnection);
	m_commandQueue->enqueue(functionChnCmd);
}

void ConfigModel::readDeviceAvailable()
{
	Command *deviceAvailableChnCmd =
		new IioDeviceAttributeRead(m_device, m_deviceAvailableAttribute.toStdString().c_str(), nullptr);
	connect(
		deviceAvailableChnCmd, &scopy::Command::finished, this,
		[=, this](scopy::Command *cmd) {
			IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
			if(!tcmd) {
				return;
			}

			if(tcmd->getReturnCode() >= 0) {
				char *result = tcmd->getResult();
				QStringList resultList = QStringList(QString(result).split(" "));
				Q_EMIT readConfigChannelDeviceAvailable(resultList);
			} else {
				qCritical(CAT_SWIOT_CONFIG)
					<< "Error: could not read attribute \"function available\" on channel"
					<< m_channelId << "error id ->" << tcmd->getReturnCode();
			}
		},
		Qt::QueuedConnection);
	m_commandQueue->enqueue(deviceAvailableChnCmd);
}

void ConfigModel::readFunctionAvailable()
{
	Command *functionAvailableChnCmd =
		new IioDeviceAttributeRead(m_device, m_functionAvailableAttribute.toStdString().c_str(), nullptr);
	connect(
		functionAvailableChnCmd, &scopy::Command::finished, this,
		[=, this](scopy::Command *cmd) {
			IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
			if(!tcmd) {
				return;
			}

			if(tcmd->getReturnCode() >= 0) {
				char *result = tcmd->getResult();
				QStringList resultList = QStringList(QString(result).split(" "));
				Q_EMIT readConfigChannelFunctionAvailable(resultList);
			} else {
				qCritical(CAT_SWIOT_CONFIG)
					<< "Error: could not read attribute \"device available\" on channel"
					<< m_channelId << "error id ->" << tcmd->getReturnCode();
			}
		},
		Qt::QueuedConnection);
	m_commandQueue->enqueue(functionAvailableChnCmd);
}
