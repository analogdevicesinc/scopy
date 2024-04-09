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

CmdQChannelAttrDataStrategy::CmdQChannelAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent)
	: QWidget(parent)
	, m_cmdQueue(recipe.connection->commandQueue())
	, m_dataRead("")
	, m_optionalDataRead("")
{
	m_recipe = recipe;
}

QString CmdQChannelAttrDataStrategy::data() { return m_dataRead; }

QString CmdQChannelAttrDataStrategy::optionalData() { return m_optionalDataRead; }

void CmdQChannelAttrDataStrategy::save(QString data)
{
	if(m_recipe.channel == nullptr || m_recipe.data == "") {
		qWarning(CAT_CMDQ_CHANNEL_DATA_STATEGY) << "Invalid arguments, cannot write any data.";
		return;
	}

	Q_EMIT aboutToWrite(m_dataRead, data);
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

	QObject::connect(readDataCommand, &Command::finished, this,
			 &CmdQChannelAttrDataStrategy::attributeReadFinished);
	m_cmdQueue->enqueue(readDataCommand);
}

void CmdQChannelAttrDataStrategy::attributeReadFinished(Command *cmd)
{
	IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
	if(!tcmd) {
		return;
	}

	if(tcmd->getReturnCode() < 0) {
		qWarning(CAT_CMDQ_CHANNEL_DATA_STATEGY) << "Could not read the value for" << m_recipe.data;
		return;
	}

	m_dataRead = QString(tcmd->getResult());
	if(!m_recipe.constDataOptions.isEmpty()) {
		m_optionalDataRead = m_recipe.constDataOptions;
		Q_EMIT sendData(m_dataRead, m_optionalDataRead);
	} else if(!m_recipe.iioDataOptions.isEmpty()) {
		Command *readOptionalCommand = new IioChannelAttributeRead(
			m_recipe.channel, m_recipe.iioDataOptions.toStdString().c_str(), nullptr);
		QObject::connect(readOptionalCommand, &Command::finished, this,
				 &CmdQChannelAttrDataStrategy::optionalAttrReadFinished);
		m_cmdQueue->enqueue(readOptionalCommand);
	} else {
		// no optional data available, emit empty string for it
		Q_EMIT sendData(m_dataRead, QString());
	}
}

void CmdQChannelAttrDataStrategy::optionalAttrReadFinished(Command *cmd)
{
	IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
	if(!tcmd) {
		return;
	}

	if(tcmd->getReturnCode() < 0) {
		qWarning(CAT_CMDQ_CHANNEL_DATA_STATEGY) << "Could not read the value for" << m_recipe.data;
		return;
	}

	char *currentOptValue = tcmd->getResult();
	m_optionalDataRead = QString(currentOptValue);
	Q_EMIT sendData(m_dataRead, m_optionalDataRead);
}

#include "moc_cmdqchannelattrdatastrategy.cpp"
