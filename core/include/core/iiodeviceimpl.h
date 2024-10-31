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

#ifndef IIODEVICEIMPL_H
#define IIODEVICEIMPL_H

#include "deviceimpl.h"

namespace scopy {

class SCOPY_CORE_EXPORT IIODeviceImpl : public DeviceImpl
{
public:
	explicit IIODeviceImpl(QString param, PluginManager *p, QObject *parent = nullptr)
		: DeviceImpl(param, p, "iio", parent)
	{}
	~IIODeviceImpl() {}

	virtual void init() override;
	bool verify() override;
	QMap<QString, QString> readDeviceInfo() override;
};

} // namespace scopy

#endif // IIODEVICEIMPL_H
