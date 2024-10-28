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

#ifndef DEVICELOADER_H
#define DEVICELOADER_H

#include "deviceimpl.h"

#include <QObject>

namespace scopy {

class DeviceLoader : public QObject
{
	Q_OBJECT
public:
	DeviceLoader(DeviceImpl *d, QObject *parent = nullptr);
	~DeviceLoader();
	void init(bool async = true);
	void asyncInit();
	void syncInit();
Q_SIGNALS:
	void initialized();

private:
	DeviceImpl *d;
	QObject *oldParent;
};
} // namespace scopy

#endif // DEVICELOADER_H
