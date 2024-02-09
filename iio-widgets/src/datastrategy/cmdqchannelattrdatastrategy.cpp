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

#include "datastrategy/cmdqchannelattrdatastrategy.h"
#include <iioutil/command.h>
#include <iioutil/iiocommand/iiochannelattributeread.h>
#include <iioutil/iiocommand/iiochannelattributewrite.h>

#define BUFFER_SIZE 16384

Q_LOGGING_CATEGORY(CAT_CMDQ_CHANNEL_DATA_STATEGY, "CmdQChannelDataStrategy")
using namespace scopy;

CmdQChannelAttrDataStrategy::CmdQChannelAttrDataStrategy(IIOWidgetFactoryRecipe recipe, CommandQueue *commandQueue,
							 QWidget *parent)
	: QWidget(parent)
	, m_cmdQueue(commandQueue)
	, m_dataRead("")
	, m_optionalDataRead("")
{
	// TODO: clean the code of this class
	m_recipe = recipe;
}

void CmdQChannelAttrDataStrategy::save(QString data)
{
	if(m_recipe.channel == nullptr || m_recipe.data == "") {
		qWarning(CAT_CMDQ_CHANNEL_DATA_STATEGY) << "Invalid arguments, cannot write any data.";
		return;
	}

	Command *writeCommand = new IioChannelAttributeWrite(m_recipe.channel, m_recipe.data.toStdString().c_str(),
							     data.toStdString().c_str(), nullptr);
	QObject::connect(
		writeCommand, &Command::finished, this,
		[data, this](Command *cmd) {
			IioChannelAttributeWrite *tcmd = dynamic_cast<IioChannelAttributeWrite *>(cmd);
			if(!tcmd) {
				return;
			}

			if(tcmd->getReturnCode() < 0) {
				qWarning(CAT_CMDQ_CHANNEL_DATA_STATEGY) << "Could not write the value " << data;
			}

			Q_EMIT emitStatus((int)(tcmd->getReturnCode()));
			requestData(); // readback
		},
		Qt::QueuedConnection);

	m_cmdQueue->enqueue(writeCommand);
}

void CmdQChannelAttrDataStrategy::requestData()
{
	if(m_recipe.channel == nullptr || m_recipe.data.isEmpty()) {
		qWarning(CAT_CMDQ_CHANNEL_DATA_STATEGY) << "Invalid arguments, cannot read any data.";
		return;
	}

	Command *readDataCommand =
		new IioChannelAttributeRead(m_recipe.channel, m_recipe.data.toStdString().c_str(), nullptr);
	Command *readOptionalCommand = nullptr;

	QObject::connect(readDataCommand, &Command::finished, this, [this](Command *cmd) {
		IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
		if(!tcmd) {
			return;
		}

		if(tcmd->getReturnCode() < 0) {
			qWarning(CAT_CMDQ_CHANNEL_DATA_STATEGY) << "Could not read the value for" << m_recipe.data;
			return;
		}

		char *currentValue = tcmd->getResult();
		std::unique_lock<std::mutex> lock(m_mutex);

		m_dataRead = QString(currentValue);

		if(m_recipe.iioDataOptions.isEmpty() && m_recipe.constDataOptions.isEmpty()) {
			Q_EMIT sendData(QString(m_dataRead), QString());
			m_dataRead.clear();

			return;
		}

		if(!m_dataRead.isEmpty() && !m_optionalDataRead.isEmpty()) {
			// emit the data collected
			Q_EMIT sendData(QString(m_dataRead), m_optionalDataRead);

			m_dataRead.clear();
			m_optionalDataRead.clear();
		}
	});

	if(!m_recipe.constDataOptions.isEmpty()) {
		// if we have const data, we should increase the op counter and check if we can emit the data
		std::unique_lock<std::mutex> lock(m_mutex);
		m_optionalDataRead = m_recipe.constDataOptions;

		if(!m_dataRead.isEmpty() && !m_optionalDataRead.isEmpty()) {
			Q_EMIT sendData(QString(m_dataRead), m_recipe.constDataOptions);

			m_dataRead.clear();
			m_optionalDataRead.clear();
		}
	} else if(!m_recipe.iioDataOptions.isEmpty()) {
		// if we have an attribute we have to read, we should read it, increase the counter and emit if possible
		readOptionalCommand = new IioChannelAttributeRead(
			m_recipe.channel, m_recipe.iioDataOptions.toStdString().c_str(), nullptr);
		QObject::connect(readOptionalCommand, &Command::finished, this, [this](Command *cmd) {
			IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
			if(!tcmd) {
				return;
			}

			if(tcmd->getReturnCode() < 0) {
				qWarning(CAT_CMDQ_CHANNEL_DATA_STATEGY)
					<< "Could not read the value for" << m_recipe.data;
				return;
			}

			char *currentOptValue = tcmd->getResult();
			std::unique_lock<std::mutex> lock(m_mutex);

			m_optionalDataRead = QString(currentOptValue);
			if(!m_dataRead.isEmpty() && !m_optionalDataRead.isEmpty()) {
				Q_EMIT sendData(QString(m_dataRead), m_optionalDataRead);

				m_dataRead.clear();
				m_optionalDataRead.clear();
			}
		});
	} else {
		std::unique_lock<std::mutex> lock(m_mutex);

		if(!m_dataRead.isEmpty()) {
			Q_EMIT sendData(QString(m_dataRead), QString());

			m_dataRead.clear();
			m_optionalDataRead.clear();
		}
	}

	m_cmdQueue->enqueue(readDataCommand);
	if(readOptionalCommand) {
		m_cmdQueue->enqueue(readOptionalCommand);
	}
}
