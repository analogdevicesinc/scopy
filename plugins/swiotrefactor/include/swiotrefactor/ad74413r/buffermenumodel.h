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

#ifndef BUFFERMENUMODEL_H
#define BUFFERMENUMODEL_H

#include "qobject.h"

#include <QMap>

#include <iioutil/commandqueue.h>
#include <iio.h>

namespace scopy::swiotrefactor {
class BufferMenuModel : public QObject
{
	Q_OBJECT
public:
	explicit BufferMenuModel(QMap<QString, iio_channel *> chnlsMap, CommandQueue *cmdQueue);
	~BufferMenuModel();

	QMap<QString, QMap<QString, QStringList>> getChnlAttrValues();
	void readChnlAttr(QString iioChannelKey, QString attrName, bool readback = false);
	void writeChnlAttr(QString iioChannelKey, QString attrName, QString attrVal,
			   QMap<QString, QMap<QString, QStringList>> newValues);
	void updateChnlAttributes(QMap<QString, QMap<QString, QStringList>> newValues, QString attrName,
				  QString chnlType);
public Q_SLOTS:
	void onChannelAttributeRead(QString iioChannelKey, QString attrName, QStringList, bool readback);
	void onChannelAttributeWritten(QString iioChannelKey, QString attrName);
Q_SIGNALS:
	void menuModelInitDone(QMap<QString, QMap<QString, QStringList>> chnlAttributes);
	void attrRead(QMap<QString, QMap<QString, QStringList>> chnlAttributes);
	void attrWritten(QMap<QString, QMap<QString, QStringList>> chnlAttributes);
	void channelAttributeRead(QString iioChannelKey, QString attrName, QStringList attrValues, bool readback);
	void channelAttributeWritten(QString iioChannelKey, QString attrName);

private:
	QMap<QString, iio_channel *> m_chnlsMap;
	QMap<QString, QMap<QString, QStringList>> m_chnlAttributes;
	CommandQueue *m_commandQueue;
	bool m_initialized;
	void init();
};
} // namespace scopy::swiotrefactor

#endif // BUFFERMENUMODEL_H
