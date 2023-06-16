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


#include "diodigitalchannelcontroller.h"
#include "src/swiot_logging_categories.h"
#include <iioutil/iiocommand/iiochannelattributeread.h>
#include <iioutil/iiocommand/iiochannelattributewrite.h>
#include <sstream>

using namespace scopy::swiot;

DioDigitalChannelController::DioDigitalChannelController(struct iio_channel* channel,
							 const QString &deviceName,
							 const QString &deviceType,
							 CommandQueue *cmdQueue,
							 QWidget *parent) :
	QWidget(parent),
	m_digitalChannel(new DioDigitalChannel(deviceName, deviceType, this)),
	m_channelName(deviceName),
	m_channelType(deviceType),
	m_cmdQueue(cmdQueue),
	m_channel(channel),
	m_writeCurrentLimitCmd(nullptr),
	m_writeTypeCmd(nullptr),
	m_writeRawCmd(nullptr),
	m_readAvailableTypeCmd(nullptr),
	m_readTypeCmd(nullptr),
	m_readRawCmd(nullptr),
	m_readCurrentLimitAvailableCmd(nullptr)
{
	m_iioAttrAvailableTypes = (m_channelType == "INPUT") ? "IEC_type_available" : "do_mode_available";
	m_iioAttrType = (m_channelType == "INPUT") ? "IEC_type" : "do_mode";


	m_readAvailableTypeCmd = new IioChannelAttributeRead(m_channel, m_iioAttrAvailableTypes.c_str(), m_cmdQueue, true);
	m_readTypeCmd = new IioChannelAttributeRead(m_channel, m_iioAttrType.c_str(), m_cmdQueue, true);

	connect(m_readAvailableTypeCmd, &scopy::Command::finished, this, [=, this](scopy::Command *cmd) {
		IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead*>(cmd);
		if (!tcmd) {
			return;
		}
		if (tcmd->getReturnCode() >= 0) {
			char *result = tcmd->getResult();
			std::stringstream ss(result);
			std::string aux;
			while (std::getline(ss, aux, ' ')) {
				m_availableTypes.push_back(aux);
			}
			this->m_digitalChannel->setConfigModes(m_availableTypes);
		} else {
			qCritical(CAT_SWIOT_MAX14906) << "Could not read the available types, error code: " << tcmd->getReturnCode();
		}
	}, Qt::QueuedConnection);

	connect(m_readTypeCmd, &scopy::Command::finished, this, [=, this](scopy::Command *cmd) {
		IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead*>(cmd);
		if (!tcmd) {
			return;
		}
		if (tcmd->getReturnCode() >= 0) {
			char *result = tcmd->getResult();
			m_type = std::string(result);
			m_digitalChannel->setSelectedConfigMode(QString::fromStdString(m_type));
		} else {
			qCritical(CAT_SWIOT_MAX14906) << "Could not read the type, error code: "  << tcmd->getReturnCode();
		}
	}, Qt::QueuedConnection);

	m_cmdQueue->enqueue(m_readAvailableTypeCmd);
	m_cmdQueue->enqueue(m_readTypeCmd);

	if (m_channelType == "OUTPUT") {
		m_readRawCmd = new IioChannelAttributeRead(m_channel, "raw", m_cmdQueue, true);
		m_readCurrentLimitAvailableCmd = new IioChannelAttributeRead(m_channel, "current_limit_available", m_cmdQueue, true);

		connect(m_readRawCmd, &scopy::Command::finished, this, [=, this](scopy::Command *cmd) {
			IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead*>(cmd);
			if (!tcmd) {
				return;
			}
			if (tcmd->getReturnCode() >= 0) {
				char *result = tcmd->getResult();
				bool ok = false;
				bool rawValue = QString(result).toInt(&ok);
				if (ok) {
					m_digitalChannel->ui->customSwitch->setChecked(rawValue);
				}
			} else {
				qCritical(CAT_SWIOT_MAX14906) <<"Could not read initial channel raw value, error code: "  << tcmd->getReturnCode();
			}
		}, Qt::QueuedConnection);

		connect(m_readCurrentLimitAvailableCmd, &scopy::Command::finished, this, [=, this](scopy::Command *cmd) {
			IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead*>(cmd);
			if (!tcmd) {
				return;
			}
			if (tcmd->getReturnCode() >= 0) {
				char *result = tcmd->getResult();
				QStringList limitAvailable = QString(result).trimmed().split(" ");
				for (const auto& item : limitAvailable) {
					this->m_digitalChannel->ui->currentLimitValues->addItem(item);
				}

				IioChannelAttributeRead *readCurrentLimitCmd = new IioChannelAttributeRead(m_channel, "current_limit", m_cmdQueue, true);
				connect(readCurrentLimitCmd, &scopy::Command::finished, this, [=, this](scopy::Command *cmd) {
					IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead*>(cmd);
					if (!tcmd) {
						return;
					}
					if (tcmd->getReturnCode() >= 0) {
						char *result = tcmd->getResult();
						m_digitalChannel->ui->currentLimitValues->setCurrentIndex(m_digitalChannel->ui->currentLimitValues->findText(result));
					} else {
						qCritical(CAT_SWIOT_MAX14906) << "Could not read current_limit, error code: "  << tcmd->getReturnCode();
					}
				}, Qt::QueuedConnection);
				m_cmdQueue->enqueue(readCurrentLimitCmd);
			} else {
				qCritical(CAT_SWIOT_MAX14906) << "Could not read current_limit_available, error code: "  << tcmd->getReturnCode();
			}
		}, Qt::QueuedConnection);

		m_cmdQueue->enqueue(m_readRawCmd);
		m_cmdQueue->enqueue(m_readCurrentLimitAvailableCmd);
	}


	QObject::connect(m_digitalChannel->ui->configModes, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 [=, this](int index){
		createWriteTypeCommand(index);
		m_cmdQueue->enqueue(m_writeTypeCmd);
	});

	QObject::connect(m_digitalChannel, &DioDigitalChannel::outputValueChanged, this,
			 [=, this] (bool value) {
		createWriteRawCommand(value);
		m_cmdQueue->enqueue(m_writeRawCmd);
	});

	QObject::connect(m_digitalChannel->ui->currentLimitValues, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 [=, this] (int index) {
		createWriteCurrentLimitCommand(index);
		m_cmdQueue->enqueue(m_writeCurrentLimitCmd);
	});
}

DioDigitalChannelController::~DioDigitalChannelController()
{

}

DioDigitalChannel *DioDigitalChannelController::getDigitalChannel() const {
	return m_digitalChannel;
}

void DioDigitalChannelController::createWriteRawCommand(bool value)
{
	m_writeRawCmd = new IioChannelAttributeWrite(m_channel, "raw", QString::number(value).toStdString().c_str(), m_cmdQueue, true);
	connect(m_writeRawCmd, &scopy::Command::finished, this, [=, this](scopy::Command *cmd) {
		IioChannelAttributeWrite *tcmd = dynamic_cast<IioChannelAttributeWrite*>(cmd);
		if (!tcmd) {
			return;
		}
		if (tcmd->getReturnCode() < 0) {
			qCritical(CAT_SWIOT_MAX14906) << "Could not write value " << value <<
							 " to channel " << m_channelName <<
							 " error code" << tcmd->getReturnCode();
		}
	}, Qt::QueuedConnection);
}

void DioDigitalChannelController::createWriteCurrentLimitCommand(int index)
{
	QString text = m_digitalChannel->ui->currentLimitValues->currentText();
	m_writeCurrentLimitCmd = new IioChannelAttributeWrite(m_channel, "current_limit", text.toStdString().c_str(), m_cmdQueue, true);
	connect(m_writeCurrentLimitCmd, &scopy::Command::finished, this, [=, this](scopy::Command *cmd) {
		IioChannelAttributeWrite *tcmd = dynamic_cast<IioChannelAttributeWrite*>(cmd);
		if (!tcmd) {
			return;
		}
		if (tcmd->getReturnCode() < 0) {
			qCritical(CAT_SWIOT_MAX14906) << "Could not write value " << text <<
							 " to channel " << m_channelName <<
							 " error code " << tcmd->getReturnCode();
		}
	}, Qt::QueuedConnection);

}

void DioDigitalChannelController::createWriteTypeCommand(int index)
{
	QString text = m_digitalChannel->ui->configModes->currentText();
	m_type = text.toStdString();
	m_writeTypeCmd = new IioChannelAttributeWrite(m_channel, m_iioAttrType.c_str(), text.toStdString().c_str(), m_cmdQueue, true);
	connect(m_writeTypeCmd, &scopy::Command::finished, this, [=, this](scopy::Command *cmd) {
		IioChannelAttributeWrite *tcmd = dynamic_cast<IioChannelAttributeWrite*>(cmd);
		if (!tcmd) {
			return;
		}
		if (tcmd->getReturnCode() < 0) {
			qCritical(CAT_SWIOT_MAX14906) << "Could not write attr to channel " << m_channelName <<
							 " error code " << tcmd->getReturnCode();
		}
	}, Qt::QueuedConnection);
}
