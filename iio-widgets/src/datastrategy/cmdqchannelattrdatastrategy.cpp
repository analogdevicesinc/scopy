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

CmdQChannelAttrDataStrategy::CmdQChannelAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent)
	: QObject(parent)
	, m_cmdQueue(recipe.connection->commandQueue())
	, m_dataRead("")
	, m_optionalDataRead("")
{
	m_recipe = recipe;
}

CmdQChannelAttrDataStrategy::~CmdQChannelAttrDataStrategy() {}

QString CmdQChannelAttrDataStrategy::data() { return m_dataRead; }

QString CmdQChannelAttrDataStrategy::optionalData() { return m_optionalDataRead; }

void CmdQChannelAttrDataStrategy::writeAsync(QString data)
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

			Q_EMIT emitStatus(QDateTime::currentDateTime(), m_dataRead, data, (int)(tcmd->getReturnCode()),
					  false);
			readAsync(); // readback
		},
		Qt::QueuedConnection);

	m_cmdQueue->enqueue(writeCommand);
}

void CmdQChannelAttrDataStrategy::readAsync()
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

int CmdQChannelAttrDataStrategy::write(QString data)
{
	qWarning(CAT_CMDQ_CHANNEL_DATA_STATEGY)
		<< "The method write(QString) was called. It signals a synchronous write, "
		   "but the command queue can only work in an async way. Consider using only "
		   "the writeAsync() method for this strategy. The return code should be "
		   "ignored.";
	return 0;
}

QPair<QString, QString> CmdQChannelAttrDataStrategy::read()
{
	qWarning(CAT_CMDQ_CHANNEL_DATA_STATEGY)
		<< "The method read() was called. It signals a synchronous read, "
		   "but the command queue can only work in an async way. Consider using only "
		   "the readAsync() method for this strategy. The result should be ignored.";
	return {};
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

	QString newData = QString(tcmd->getResult());
	if(!m_recipe.constDataOptions.isEmpty()) {
		QString oldData = m_dataRead;
		m_optionalDataRead = m_recipe.constDataOptions;
		m_dataRead = newData;
		Q_EMIT emitStatus(QDateTime::currentDateTime(), oldData, m_dataRead, tcmd->getReturnCode(), true);
		Q_EMIT sendData(m_dataRead, m_optionalDataRead);
	} else if(!m_recipe.iioDataOptions.isEmpty()) {
		m_dataRead = newData;
		Command *readOptionalCommand = new IioChannelAttributeRead(
			m_recipe.channel, m_recipe.iioDataOptions.toStdString().c_str(), nullptr);
		QObject::connect(readOptionalCommand, &Command::finished, this,
				 &CmdQChannelAttrDataStrategy::optionalAttrReadFinished);
		m_cmdQueue->enqueue(readOptionalCommand);
	} else {
		QString oldData = m_dataRead;
		m_dataRead = newData;
		Q_EMIT emitStatus(QDateTime::currentDateTime(), oldData, m_dataRead, tcmd->getReturnCode(), true);
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
	QString oldData = m_optionalDataRead;
	m_optionalDataRead = QString(currentOptValue);

	Q_EMIT emitStatus(QDateTime::currentDateTime(), oldData, m_optionalDataRead, tcmd->getReturnCode(), true);
	Q_EMIT sendData(m_dataRead, m_optionalDataRead);
}

#include "moc_cmdqchannelattrdatastrategy.cpp"
