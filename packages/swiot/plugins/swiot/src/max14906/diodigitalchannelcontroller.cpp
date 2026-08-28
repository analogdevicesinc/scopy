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

#include <component/channel.h>
#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/attributewriter.h>

#include <qcorotask.h>

using namespace scopy::swiot;

DioDigitalChannelController::DioDigitalChannelController(component::Channel *channel, const QString &deviceName,
							 const QString &deviceType, QWidget *parent)
	: QWidget(parent)
	, m_digitalChannel(new DioDigitalChannel(deviceName, deviceType, this))
	, m_channelName(deviceName)
	, m_channelType(deviceType)
	, m_channel(channel)
	, m_typeAttr(nullptr)
	, m_rawAttr(nullptr)
	, m_currentLimitAttr(nullptr)
{
	initChannelAttributes();

	connect(m_digitalChannel->m_configModesCombo->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, [=, this](int index) { createWriteTypeCommand(index); });

	connect(m_digitalChannel, &DioDigitalChannel::outputValueChanged, this,
		[=, this](bool value) { createWriteRawCommand(value); });

	connect(m_digitalChannel->m_currentLimitsCombo->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, [=, this](int index) { createWriteCurrentLimitCommand(index); });
}

DioDigitalChannelController::~DioDigitalChannelController() {}

DioDigitalChannel *DioDigitalChannelController::getDigitalChannel() const { return m_digitalChannel; }

void DioDigitalChannelController::initChannelAttributes()
{
	if(!m_channel) {
		return;
	}

	QString typeAttrName = (m_channelType == "INPUT") ? "IEC_type" : "do_mode";
	m_typeAttr = m_channel->findChild<component::Attribute *>(typeAttrName, Qt::FindDirectChildrenOnly);

	if(m_typeAttr) {
		m_availableTypes = m_typeAttr->options();
		m_digitalChannel->setConfigModes(m_availableTypes);
		if(m_typeAttr->readCapability()) {
			QCoro::waitFor(m_typeAttr->readCapability()->readAsync());
			m_type = m_typeAttr->cachedValue();
			m_digitalChannel->setSelectedConfigMode(m_type);
		}
	} else {
		qCritical(CAT_SWIOT_MAX14906) << "Could not find the type attribute for channel" << m_channelName;
	}

	if(m_channelType == "OUTPUT") {
		m_currentLimitAttr =
			m_channel->findChild<component::Attribute *>("current_limit", Qt::FindDirectChildrenOnly);
		if(m_currentLimitAttr) {
			const QStringList limitAvailable = m_currentLimitAttr->options();
			for(const auto &item : limitAvailable) {
				m_digitalChannel->m_currentLimitsCombo->combo()->addItem(item);
			}
			if(m_currentLimitAttr->readCapability()) {
				QCoro::waitFor(m_currentLimitAttr->readCapability()->readAsync());
				QString limit = m_currentLimitAttr->cachedValue();
				int idx = m_digitalChannel->m_currentLimitsCombo->combo()->findText(limit);
				if(idx < 0) {
					qCritical(CAT_SWIOT_MAX14906) << "Could not find the " << limit
								      << " in current limit available values!";
				}
				m_digitalChannel->m_currentLimitsCombo->combo()->setCurrentIndex(idx);
			}
		}

		m_rawAttr = m_channel->findChild<component::Attribute *>("raw", Qt::FindDirectChildrenOnly);
		if(m_rawAttr && m_rawAttr->readCapability()) {
			QCoro::waitFor(m_rawAttr->readCapability()->readAsync());
			bool ok = false;
			bool rawValue = m_rawAttr->cachedValue().toInt(&ok);
			if(ok) {
				m_digitalChannel->m_valueSwitch->setChecked(rawValue);
			}
		}
	}
}

void DioDigitalChannelController::createWriteRawCommand(bool value)
{
	if(!m_rawAttr || !m_rawAttr->writeCapability()) {
		return;
	}
	m_rawAttr->writeCapability()->writeAsync(QString::number(value));
}

void DioDigitalChannelController::createWriteCurrentLimitCommand(int index)
{
	if(!m_currentLimitAttr || !m_currentLimitAttr->writeCapability()) {
		return;
	}
	QString text = m_digitalChannel->m_currentLimitsCombo->combo()->currentText();
	m_currentLimitAttr->writeCapability()->writeAsync(text);
}

void DioDigitalChannelController::createWriteTypeCommand(int index)
{
	if(!m_typeAttr || !m_typeAttr->writeCapability()) {
		return;
	}
	QString text = m_digitalChannel->m_configModesCombo->combo()->currentText();
	m_type = text;
	m_typeAttr->writeCapability()->writeAsync(text);
}

#include "moc_diodigitalchannelcontroller.cpp"
