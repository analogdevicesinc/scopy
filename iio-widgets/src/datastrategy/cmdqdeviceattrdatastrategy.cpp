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

#include "datastrategy/cmdqdeviceattrdatastrategy.h"
#include <iioutil/command.h>
#include <iioutil/iiocommand/iiodeviceattributeread.h>
#include <iioutil/iiocommand/iiodeviceattributewrite.h>
#include <QLoggingCategory>

#define BUFFER_SIZE 16384
Q_LOGGING_CATEGORY(CAT_CMDQ_DEVICE_DATA_STRATEGY, "CmdQDeviceDataStrategy")

using namespace scopy;

CmdQDeviceAttrDataStrategy::CmdQDeviceAttrDataStrategy(IIOWidgetFactoryRecipe recipe, CommandQueue *commandQueue,
						       QWidget *parent)
	: QWidget(parent)
	, m_cmdQueue(commandQueue)
	, m_dataRead("")
	, m_optionalDataRead("")
{
	m_recipe = recipe;
}

void CmdQDeviceAttrDataStrategy::save(QString data)
{
	if(m_recipe.device == nullptr || m_recipe.data == "") {
		qWarning(CAT_CMDQ_DEVICE_DATA_STRATEGY) << "Invalid arguments, cannot write any data.";
		return;
	}

	Command *writeCommand = new IioDeviceAttributeWrite(m_recipe.device, m_recipe.data.toStdString().c_str(),
							    data.toStdString().c_str(), nullptr);
	QObject::connect(
		writeCommand, &Command::finished, this,
		[data, this](Command *cmd) {
			IioDeviceAttributeWrite *tcmd = dynamic_cast<IioDeviceAttributeWrite *>(cmd);
			if(!tcmd) {
				return;
			}

			if(tcmd->getReturnCode() < 0) {
				qWarning(CAT_CMDQ_DEVICE_DATA_STRATEGY) << "Could not write the value " << data;
			}

			Q_EMIT emitStatus((int)(tcmd->getReturnCode()));
			requestData(); // readback
		},
		Qt::QueuedConnection);

	m_cmdQueue->enqueue(writeCommand);
}

void CmdQDeviceAttrDataStrategy::requestData()
{
	if(m_recipe.device == nullptr || m_recipe.data.isEmpty()) {
		qWarning(CAT_CMDQ_DEVICE_DATA_STRATEGY) << "Invalid arguments, cannot read any data.";
		return;
	}

	Command *readDataCommand =
		new IioDeviceAttributeRead(m_recipe.device, m_recipe.data.toStdString().c_str(), nullptr);

	QObject::connect(readDataCommand, &Command::finished, this, [this](Command *cmd) {
		IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
		if(!tcmd) {
			return;
		}

		if(tcmd->getReturnCode() < 0) {
			qWarning(CAT_CMDQ_DEVICE_DATA_STRATEGY) << "Could not read the value for" << m_recipe.data;
			return;
		}

		char *currentValue = tcmd->getResult();
		m_dataRead = QString(currentValue);

		if(!m_recipe.constDataOptions.isEmpty()) {
			Q_EMIT sendData(m_dataRead, m_recipe.constDataOptions);

			m_dataRead.clear();
			m_optionalDataRead.clear();
		} else if(!m_recipe.iioDataOptions.isEmpty()) {
			// if we have an attribute we have to read, we should read it, increase the counter and emit if
			// possible
			Command *readOptionalCommand = new IioDeviceAttributeRead(
				m_recipe.device, m_recipe.iioDataOptions.toStdString().c_str(), nullptr);
			QObject::connect(readOptionalCommand, &Command::finished, this, [this](Command *cmd) {
				IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
				if(!tcmd) {
					return;
				}

				if(tcmd->getReturnCode() < 0) {
					qWarning(CAT_CMDQ_DEVICE_DATA_STRATEGY)
						<< "Could not read the value for" << m_recipe.data;
					return;
				}

				char *currentOptValue = tcmd->getResult();
				m_optionalDataRead = QString(currentOptValue);

				if(!m_dataRead.isEmpty() && !m_optionalDataRead.isEmpty()) {
					Q_EMIT sendData(m_dataRead, m_optionalDataRead);

					m_dataRead.clear();
					m_optionalDataRead.clear();
				}
			});
			m_cmdQueue->enqueue(readOptionalCommand);
		} else {
			// no optional data available, emit empty string for it
			Q_EMIT sendData(m_dataRead, QString());

			m_dataRead.clear();
			m_optionalDataRead.clear();
		}
	});
	m_cmdQueue->enqueue(readDataCommand);
}
