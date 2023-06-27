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

extern "C"{
struct iio_channel;
}

namespace scopy::swiot {
class BufferMenuModel: public QObject{
	Q_OBJECT
public:
	explicit BufferMenuModel(QMap<QString, iio_channel*> chnlsMap = {});
	~BufferMenuModel();

	QMap<QString, QMap<QString, QStringList>> getChnlAttrValues();
	QStringList readChnlAttr(struct iio_channel* iio_chnl, QString attrName);

	void init();
	void updateChnlAttributes(QMap<QString, QMap<QString,QStringList>> newValues, QString attrName, QString chnlType);
Q_SIGNALS:
	void attrWritten(QString value);
private:
	QMap<QString, iio_channel*> m_chnlsMap;
	QMap<QString, QMap<QString, QStringList>> m_chnlAttributes;

};
}

#endif // BUFFERMENUMODEL_H
