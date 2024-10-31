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
 *
 */

#ifndef XMLFILEMANAGER_HPP
#define XMLFILEMANAGER_HPP

#include "registermaptemplate.hpp"
#include "scopy-regmap_export.h"

#include <iio.h>

#include <QObject>

class QDomElement;
class QString;

namespace scopy::regmap {
class RegisterModel;
class BitFieldModel;

class SCOPY_REGMAP_EXPORT XmlFileManager : public QObject
{
	Q_OBJECT

public:
	XmlFileManager(struct iio_device *dev, QString filePath);

	QList<QString> *getAllAddresses();
	QMap<uint32_t, RegisterModel *> *getAllRegisters(RegisterMapTemplate *parent);
	RegisterModel *getRegAtAddress(QString addr);
	RegisterModel *getRegister(QDomElement reg, RegisterMapTemplate *parent);
	void getRegMapData(QString addr);
	QVector<BitFieldModel *> *getBitFields(QString regAddr);
	QVector<BitFieldModel *> *getBitFieldsOfRegister(QDomElement reg, bool reverseBitOrder);
	BitFieldModel *getBitField(QDomElement bitField);

private:
	QString filePath;
	struct iio_device *dev;
};
} // namespace scopy::regmap
#endif // XMLFILEMANAGER_HPP
