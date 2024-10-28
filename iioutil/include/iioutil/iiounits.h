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

#ifndef IIOUNITS_H
#define IIOUNITS_H

#include <QString>
#include <QMap>
#include <QObject>

#include "scopy-iioutil_export.h"
#include <iio.h>

namespace scopy {

typedef struct IIOUnit_
{
	QString name;
	QString symbol;
	double scale = 1;
} IIOUnit;

class SCOPY_IIOUTIL_EXPORT IIOUnitsManager : public QObject
{
	Q_OBJECT
protected:
	IIOUnitsManager(QObject *parent = nullptr);
	~IIOUnitsManager();

public:
	// singleton
	IIOUnitsManager(IIOUnitsManager &other) = delete;
	void operator=(const IIOUnitsManager &) = delete;
	static IIOUnitsManager *GetInstance();

	static QMap<iio_chan_type, IIOUnit> iioChannelTypes();
	static QMap<hwmon_chan_type, IIOUnit> hwmonChannelTypes();

private:
	QMap<iio_chan_type, IIOUnit> _iioChannelTypes();
	QMap<hwmon_chan_type, IIOUnit> _hwmonChannelTypes();

private:
	static IIOUnitsManager *pinstance_;
	QMap<iio_chan_type, IIOUnit> m_iioChannelTypes;
	QMap<hwmon_chan_type, IIOUnit> m_hwmonChannelTypes;
};

}; // namespace scopy

#endif // IIOUNITS_H
