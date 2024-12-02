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


#include <iio.h>
#include "qdebug.h"
#include "buffermenumodel.h"
#include <iioutil/iiocommand/iiochannelattributeread.h>
#include <iioutil/iiocommand/iiochannelattributewrite.h>

using namespace scopy::swiot;

BufferMenuModel::BufferMenuModel(QMap<QString, iio_channel*> chnlsMap, CommandQueue *cmdQueue)
	: m_chnlsMap(chnlsMap)
	, m_commandQueue(cmdQueue)
{
	connect(this, &BufferMenuModel::channelAttributeRead, this, &BufferMenuModel::onChannelAttributeRead);
	connect(this, &BufferMenuModel::channelAttributeWritten, this, &BufferMenuModel::onChannelAttributeWritten);
	init();
}

BufferMenuModel::~BufferMenuModel()
{}

void BufferMenuModel::init()
{
	if (m_chnlsMap.size() > 0) {
		auto keys = m_chnlsMap.keys();
		for (const QString &key : keys) {
			int chnlAttrNumber = iio_channel_get_attrs_count(m_chnlsMap[key]);
			QStringList attrValues;
			for (int i = 0; i < chnlAttrNumber; i++) {
				QString attrName(iio_channel_get_attr(m_chnlsMap[key], i));
				readChnlAttr(key, attrName);

			}
		}
	}
}

QMap<QString, QMap<QString, QStringList>> BufferMenuModel::getChnlAttrValues()
{
	return m_chnlAttributes;
}


void BufferMenuModel::onChannelAttributeRead(QString iioChannelKey, QString attrName, QStringList attrValues)
{
	m_chnlAttributes[iioChannelKey][attrName] = attrValues;
}

void BufferMenuModel::onChannelAttributeWritten(QString iioChannelKey, QString attrName)
{
	// perform a readback
	readChnlAttr(iioChannelKey, attrName);
	Q_EMIT attrWritten(m_chnlAttributes);
}

void BufferMenuModel::readChnlAttr(QString iioChannelKey, QString attrName)
{
	Command *channelAttrReadCommand = new IioChannelAttributeRead(m_chnlsMap[iioChannelKey], attrName.toStdString().c_str(), nullptr);
	connect(channelAttrReadCommand, &scopy::Command::finished, this, [=, this] (scopy::Command* cmd) {
		IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead*>(cmd);
		if (!tcmd) { return; }

		if (tcmd->getReturnCode() >= 0) {
			char *result = tcmd->getResult();
			QString bufferValues(result);
			QStringList attrValues = bufferValues.split(" ");
			Q_EMIT channelAttributeRead(iioChannelKey, attrName, attrValues);
		}
	}, Qt::QueuedConnection);
	m_commandQueue->enqueue(channelAttrReadCommand);
}

void BufferMenuModel::writeChnlAttr(QString iioChannelKey, QString attrName, QString attrVal,
				    QMap<QString, QMap<QString,QStringList>> newValues)
{
	Command *channelAttrWriteCommand = new IioChannelAttributeWrite(m_chnlsMap[iioChannelKey], attrName.toStdString().c_str(),
									attrVal.toStdString().c_str(), nullptr);
	connect(channelAttrWriteCommand, &scopy::Command::finished, this, [=, this] (scopy::Command* cmd) {
		IioChannelAttributeWrite *tcmd = dynamic_cast<IioChannelAttributeWrite*>(cmd);
		if (!tcmd) { return; }

		if (tcmd->getReturnCode() >= 0) {
			m_chnlAttributes = newValues;
			Q_EMIT channelAttributeWritten(iioChannelKey, attrName);
		}
	}, Qt::QueuedConnection);
	m_commandQueue->enqueue(channelAttrWriteCommand);
}

void BufferMenuModel::updateChnlAttributes(QMap<QString, QMap<QString,QStringList>> newValues,
					   QString attrName, QString chnlType)
{
	QStringList value = newValues[chnlType].value(attrName);
	if (value.size() == 1) {
		QString attrVal = value.first();
		std::string s_attrValue = attrVal.toStdString();
		std::string s_attrName = attrName.toStdString();
		if (m_chnlsMap.contains(chnlType) && m_chnlsMap[chnlType] != nullptr) {
			writeChnlAttr(chnlType, s_attrName.c_str(), s_attrValue.c_str(), newValues);
		}
	}

}

