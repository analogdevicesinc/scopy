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

using namespace scopy::swiot;

BufferMenuModel::BufferMenuModel(QMap<QString, iio_channel*> chnlsMap):
	m_chnlsMap(chnlsMap)
{
	init();
}

BufferMenuModel::~BufferMenuModel()
{}

void BufferMenuModel::init()
{
	if (m_chnlsMap.size() > 0) {
		for (const QString &key : m_chnlsMap.keys()) {
			int chnlAttrNumber = iio_channel_get_attrs_count(m_chnlsMap[key]);
			QStringList attrValues;
			for (int i = 0; i < chnlAttrNumber; i++) {
				QString attrName(iio_channel_get_attr(m_chnlsMap[key], i));
				attrValues = readChnlAttr(m_chnlsMap[key], attrName);
				m_chnlAttributes[key][attrName] = attrValues;
				attrValues.clear();
			}
		}
	}
}

QMap<QString, QMap<QString, QStringList>> BufferMenuModel::getChnlAttrValues()
{
	return m_chnlAttributes;
}

QStringList BufferMenuModel::readChnlAttr(struct iio_channel* iio_chnl, QString attrName)
{
	QStringList attrValues;
	char* buffer = new char[200];
	std::string s_attrName = attrName.toStdString();
	int returnCode = iio_channel_attr_read(iio_chnl, s_attrName.c_str(), buffer, 199);

	if (returnCode >= 0) {
		QString bufferValues(buffer);
		attrValues = bufferValues.split(" ");
	}

	attrValues.removeAll("");
	delete[] buffer;
	return attrValues;
}

void BufferMenuModel::updateChnlAttributes(QMap<QString, QMap<QString,QStringList>> newValues, QString attrName, QString chnlType)
{
	QStringList value = newValues[chnlType].value(attrName);
	if (value.size() == 1) {
		QString attrVal = value.first();
		std::string s_attrValue = attrVal.toStdString();
		std::string s_attrName = attrName.toStdString();
		if (m_chnlsMap.contains(chnlType) && m_chnlsMap[chnlType] != nullptr) {
			int retCode = iio_channel_attr_write(m_chnlsMap[chnlType], s_attrName.c_str(), s_attrValue.c_str());
			if (retCode >= 0) {
				m_chnlAttributes = newValues;
				qInfo() << attrName.toUpper() +" was successfully written!";
			} else {
				qWarning() << "Couldn't write " + attrName.toUpper() + " attribute! (" + QString::number(retCode) + ")";
			}
			QStringList readBack = readChnlAttr(m_chnlsMap[chnlType], attrName);
			if (readBack.size() == 1) {
				m_chnlAttributes[chnlType][attrName] = readBack;
				Q_EMIT attrWritten(m_chnlAttributes);
			}
		}
	}

}

