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

#ifndef DEVICEICON_H
#define DEVICEICON_H
#include "scopy-core_export.h"

#include <QAbstractButton>

#include <device.h>

namespace scopy {
class SCOPY_CORE_EXPORT DeviceIcon : public QAbstractButton
{
	Q_OBJECT
public:
	DeviceIcon(QWidget *parent = nullptr)
		: QAbstractButton(parent){};
	virtual ~DeviceIcon(){};
public Q_SLOTS:
	//	virtual Device* device() = 0;
	virtual void setConnected(bool) = 0;
	virtual void setConnecting(bool) = 0;
Q_SIGNALS:
	void refresh();
	void forget();
};
} // namespace scopy

#endif // DEVICEICON_H
