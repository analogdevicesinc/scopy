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

#include "src/swiot_logging_categories.h"

#include <iioutil/commandqueueprovider.h>
#include <iioutil/contextprovider.h>
#include <iioutil/iiocommand/iiochannelattributeread.h>
#include <utility>

#define DEVICE_NAME "adt75"
#define CHANNEL_NAME "temp"

using namespace scopy::swiot;

SwiotReadTemperatureTask::SwiotReadTemperatureTask(QString uri, QObject *parent)
	: QThread(parent)
	, m_uri(std::move(uri))
	, m_context(nullptr)
	, m_channel(nullptr)
	, m_device(nullptr)
	, m_commandQueue(nullptr)
	, m_raw(0.0)
	, m_scale(0.0)
	, m_offset(0.0)
{
	m_context = ContextProvider::GetInstance()->open(m_uri);

	if(!m_context) {
		qDebug(CAT_SWIOT) << "Error, empty context received by temperature task.";
		return;
	}

	m_device = iio_context_find_device(m_context, DEVICE_NAME);
	if(!m_device) {
		qDebug(CAT_SWIOT) << "Error, could not find" << DEVICE_NAME
				  << "from the given context. Temperature not available.";
		ContextProvider::GetInstance()->close(m_uri);
		return;
	}

	if(isInterruptionRequested()) {
		return;
	}

	m_commandQueue = CommandQueueProvider::GetInstance()->open(m_context);
	if(!m_commandQueue) {
		ContextProvider::GetInstance()->close(m_uri);
		return;
	}

	m_channel = iio_device_find_channel(m_device, CHANNEL_NAME, false);
	if(!m_channel) {
		qDebug(CAT_SWIOT) << "Error, could not find channel " << CHANNEL_NAME << "from device" << DEVICE_NAME
				  << ". Temperature not available.";
		ContextProvider::GetInstance()->close(m_uri);
		return;
	}
	Command *attrReadScale = new IioChannelAttributeRead(m_channel, "scale", nullptr, true);
	Command *attrReadOffset = new IioChannelAttributeRead(m_channel, "offset", nullptr, true);

	connect(attrReadScale, &scopy::Command::finished, this, &SwiotReadTemperatureTask::readScaleCommandFinished,
		Qt::QueuedConnection);
	connect(attrReadOffset, &scopy::Command::finished, this, &SwiotReadTemperatureTask::readOffsetCommandFinished,
		Qt::QueuedConnection);
	m_commandQueue->enqueue(attrReadScale);
	m_commandQueue->enqueue(attrReadOffset);
}

SwiotReadTemperatureTask::~SwiotReadTemperatureTask() { m_commandQueue = nullptr; }

void SwiotReadTemperatureTask::run()
{
	if(isInterruptionRequested()) {
		return;
	}

	Command *attrReadRaw = new IioChannelAttributeRead(m_channel, "raw", nullptr);
	connect(attrReadRaw, &scopy::Command::finished, this, &SwiotReadTemperatureTask::readRawCommandFinished,
		Qt::QueuedConnection);

	m_commandQueue->enqueue(attrReadRaw);
}

void SwiotReadTemperatureTask::readRawCommandFinished(Command *cmd)
{
	if(isInterruptionRequested()) {
		ContextProvider::GetInstance()->close(m_uri);
		return;
	}
	IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
	if(!tcmd) {
		CommandQueueProvider::GetInstance()->close(m_context);
		ContextProvider::GetInstance()->close(m_uri);
		return;
	}
	if(tcmd->getReturnCode() >= 0) {
		char *res = tcmd->getResult();
		bool ok = false;
		double raw = QString(res).toDouble(&ok);
		if(ok) {
			m_raw = raw;
			double temperature = (m_raw + m_offset) * m_scale / 1000;
			qDebug(CAT_SWIOT) << "Read temperature value of" << temperature;
			Q_EMIT newTemperature(temperature);
		}
	} else {
		qDebug(CAT_SWIOT) << "Error, could not read \"raw\" attribute from " << DEVICE_NAME
				  << ". Temperature not available.";
		CommandQueueProvider::GetInstance()->close(m_context);
		ContextProvider::GetInstance()->close(m_uri);
	}
}

void SwiotReadTemperatureTask::readScaleCommandFinished(Command *cmd)
{
	IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
	if(!tcmd) {
		CommandQueueProvider::GetInstance()->close(m_context);
		ContextProvider::GetInstance()->close(m_uri);
		return;
	}
	if(tcmd->getReturnCode() >= 0) {
		char *res = tcmd->getResult();
		bool ok = false;
		double scale = QString(res).toDouble(&ok);
		if(ok) {
			m_scale = scale;
		}
	} else {
		qDebug(CAT_SWIOT) << "Error, could not read \"scale\" attribute from " << DEVICE_NAME
				  << ". Temperature not available.";
		CommandQueueProvider::GetInstance()->close(m_context);
		ContextProvider::GetInstance()->close(m_uri);
	}
}

void SwiotReadTemperatureTask::readOffsetCommandFinished(Command *cmd)
{
	IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
	if(!tcmd) {
		CommandQueueProvider::GetInstance()->close(m_context);
		ContextProvider::GetInstance()->close(m_uri);
		return;
	}
	if(tcmd->getReturnCode() >= 0) {
		char *res = tcmd->getResult();
		bool ok = false;
		double offset = QString(res).toDouble(&ok);
		if(ok) {
			m_offset = offset;
		}
	} else {
		qDebug(CAT_SWIOT) << "Error, could not read \"offset\" attribute from " << DEVICE_NAME
				  << ". Temperature not available.";
	}
	CommandQueueProvider::GetInstance()->close(m_context);
	ContextProvider::GetInstance()->close(m_uri);
}

#include "moc_swiotreadtemperaturetask.cpp"
