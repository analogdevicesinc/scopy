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

#include "max14906/diodigitalchannelcontroller.h"

#include "swiot_logging_categories.h"

#include <iioutil/iiocommand/iiochannelattributeread.h>
#include <iioutil/iiocommand/iiochannelattributewrite.h>

using namespace scopy::swiotrefactor;

DioDigitalChannelController::DioDigitalChannelController(struct iio_channel *channel, const QString &deviceName,
							 const QString &deviceType, CommandQueue *cmdQueue,
							 QWidget *parent)
	: QWidget(parent)
	, m_digitalChannel(new DioDigitalChannel(deviceName, deviceType, this))
	, m_channelName(deviceName)
	, m_channelType(deviceType)
	, m_cmdQueue(cmdQueue)
	, m_channel(channel)
{
	m_iioAttrAvailableTypes = (m_channelType == "INPUT") ? "IEC_type_available" : "do_mode_available";
	m_iioAttrType = (m_channelType == "INPUT") ? "IEC_type" : "do_mode";

	Command *readAvailableTypeCmd =
		new IioChannelAttributeRead(m_channel, m_iioAttrAvailableTypes.toStdString().c_str(), nullptr);
	Command *readTypeCmd = new IioChannelAttributeRead(m_channel, m_iioAttrType.toStdString().c_str(), nullptr);

	connect(readAvailableTypeCmd, &scopy::Command::finished, this,
		&DioDigitalChannelController::readAvailableTypeCmdFinished, Qt::QueuedConnection);

	connect(readTypeCmd, &scopy::Command::finished, this, &DioDigitalChannelController::readTypeCmdFinished,
		Qt::QueuedConnection);

	m_cmdQueue->enqueue(readAvailableTypeCmd);
	m_cmdQueue->enqueue(readTypeCmd);

	if(m_channelType == "OUTPUT") {
		Command *readRawCmd = new IioChannelAttributeRead(m_channel, "raw", nullptr);
		Command *readCurrentLimitAvailableCmd =
			new IioChannelAttributeRead(m_channel, "current_limit_available", nullptr);

		connect(readRawCmd, &scopy::Command::finished, this, &DioDigitalChannelController::readRawCmdFinished,
			Qt::QueuedConnection);

		connect(readCurrentLimitAvailableCmd, &scopy::Command::finished, this,
			&DioDigitalChannelController::readCurrentLimitAvailableCmdFinished, Qt::QueuedConnection);

		m_cmdQueue->enqueue(readRawCmd);
		m_cmdQueue->enqueue(readCurrentLimitAvailableCmd);
	}

	connect(m_digitalChannel->m_configModesCombo->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, [=, this](int index) { createWriteTypeCommand(index); });

	connect(m_digitalChannel, &DioDigitalChannel::outputValueChanged, this,
		[=, this](bool value) { createWriteRawCommand(value); });

	connect(m_digitalChannel->m_currentLimitsCombo->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, [=, this](int index) { createWriteCurrentLimitCommand(index); });
}

DioDigitalChannelController::~DioDigitalChannelController() {}

DioDigitalChannel *DioDigitalChannelController::getDigitalChannel() const { return m_digitalChannel; }

void DioDigitalChannelController::createWriteRawCommand(bool value)
{
	Command *writeRawCmd =
		new IioChannelAttributeWrite(m_channel, "raw", QString::number(value).toStdString().c_str(), nullptr);
	connect(
		writeRawCmd, &scopy::Command::finished, this,
		[=, this](scopy::Command *cmd) {
			IioChannelAttributeWrite *tcmd = dynamic_cast<IioChannelAttributeWrite *>(cmd);
			if(!tcmd) {
				return;
			}
			if(tcmd->getReturnCode() < 0) {
				qCritical(CAT_SWIOT_MAX14906)
					<< "Could not write value " << value << " to channel " << m_channelName
					<< " error code" << tcmd->getReturnCode();
			}
		},
		Qt::QueuedConnection);
	m_cmdQueue->enqueue(writeRawCmd);
}

void DioDigitalChannelController::createWriteCurrentLimitCommand(int index)
{
	QString text = m_digitalChannel->m_currentLimitsCombo->combo()->currentText();
	Command *writeCurrentLimitCmd =
		new IioChannelAttributeWrite(m_channel, "current_limit", text.toStdString().c_str(), nullptr);
	connect(writeCurrentLimitCmd, &scopy::Command::finished, this,
		&DioDigitalChannelController::writeCurrentLimitCmdFinished, Qt::QueuedConnection);
	m_cmdQueue->enqueue(writeCurrentLimitCmd);
}

void DioDigitalChannelController::createWriteTypeCommand(int index)
{
	QString text = m_digitalChannel->m_configModesCombo->combo()->currentText();
	m_type = text;
	Command *writeTypeCmd = new IioChannelAttributeWrite(m_channel, m_iioAttrType.toStdString().c_str(),
							     text.toStdString().c_str(), nullptr);
	connect(writeTypeCmd, &scopy::Command::finished, this, &DioDigitalChannelController::writeTypeCmdFinished,
		Qt::QueuedConnection);
	m_cmdQueue->enqueue(writeTypeCmd);
}

void DioDigitalChannelController::readAvailableTypeCmdFinished(Command *cmd)
{
	IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
	if(!tcmd) {
		return;
	}
	if(tcmd->getReturnCode() >= 0) {
		QString result(tcmd->getResult());
		m_availableTypes = result.trimmed().split(" ");
		m_digitalChannel->setConfigModes(m_availableTypes);
	} else {
		qCritical(CAT_SWIOT_MAX14906)
			<< "Could not read the available types, error code: " << tcmd->getReturnCode();
	}
}

void DioDigitalChannelController::readTypeCmdFinished(Command *cmd)
{
	IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
	if(!tcmd) {
		return;
	}
	if(tcmd->getReturnCode() >= 0) {
		char *result = tcmd->getResult();
		m_type = result;
		m_digitalChannel->setSelectedConfigMode(m_type);
	} else {
		qCritical(CAT_SWIOT_MAX14906) << "Could not read the type, error code: " << tcmd->getReturnCode();
	}
}

void DioDigitalChannelController::readRawCmdFinished(Command *cmd)
{
	IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
	if(!tcmd) {
		return;
	}
	if(tcmd->getReturnCode() >= 0) {
		char *result = tcmd->getResult();
		bool ok = false;
		bool rawValue = QString(result).toInt(&ok);
		if(ok) {
			m_digitalChannel->m_valueSwitch->setChecked(rawValue);
		}
	} else {
		qCritical(CAT_SWIOT_MAX14906)
			<< "Could not read initial channel raw value, error code: " << tcmd->getReturnCode();
	}
}

void DioDigitalChannelController::readCurrentLimitCmdFinished(Command *cmd)
{
	IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
	if(!tcmd) {
		return;
	}
	if(tcmd->getReturnCode() >= 0) {
		char *result = tcmd->getResult();
		int idx = m_digitalChannel->m_currentLimitsCombo->combo()->findText(result);
		if(idx < 0) {
			qCritical(CAT_SWIOT_MAX14906)
				<< "Could not find the " << result << " in current limit available values!";
		}
		m_digitalChannel->m_currentLimitsCombo->combo()->setCurrentIndex(idx);
	} else {
		qCritical(CAT_SWIOT_MAX14906) << "Could not read current_limit, error code: " << tcmd->getReturnCode();
	}
}

void DioDigitalChannelController::readCurrentLimitAvailableCmdFinished(Command *cmd)
{
	IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
	if(!tcmd) {
		return;
	}
	if(tcmd->getReturnCode() >= 0) {
		char *result = tcmd->getResult();
		QStringList limitAvailable = QString(result).trimmed().split(" ");
		for(const auto &item : limitAvailable) {
			m_digitalChannel->m_currentLimitsCombo->combo()->addItem(item);
		}

		IioChannelAttributeRead *readCurrentLimitCmd =
			new IioChannelAttributeRead(m_channel, "current_limit", nullptr);
		connect(readCurrentLimitCmd, &scopy::Command::finished, this,
			&DioDigitalChannelController::readCurrentLimitCmdFinished, Qt::QueuedConnection);
		m_cmdQueue->enqueue(readCurrentLimitCmd);
	} else {
		qCritical(CAT_SWIOT_MAX14906)
			<< "Could not read current_limit_available, error code: " << tcmd->getReturnCode();
	}
}

void DioDigitalChannelController::writeCurrentLimitCmdFinished(Command *cmd)
{
	IioChannelAttributeWrite *tcmd = dynamic_cast<IioChannelAttributeWrite *>(cmd);
	if(!tcmd) {
		return;
	}
	if(tcmd->getReturnCode() < 0) {
		qCritical(CAT_SWIOT_MAX14906) << "Could not write current limit value to channel " << m_channelName
					      << " error code " << tcmd->getReturnCode();
	}
}

void DioDigitalChannelController::writeTypeCmdFinished(Command *cmd)
{
	IioChannelAttributeWrite *tcmd = dynamic_cast<IioChannelAttributeWrite *>(cmd);
	if(!tcmd) {
		return;
	}
	if(tcmd->getReturnCode() < 0) {
		qCritical(CAT_SWIOT_MAX14906) << "Could not write attr to channel " << m_channelName << " error code "
					      << tcmd->getReturnCode();
	}
}
