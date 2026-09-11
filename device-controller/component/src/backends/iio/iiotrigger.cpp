/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "component/backends/iio/iiotrigger.h"

#include "iioutil/ideviceops.h"

using namespace scopy::component::iio;

IIOTrigger::IIOTrigger(scopy::iio::IDeviceOps *ops, scopy::iio::DeviceHandle dev, QObject *parent)
	: QObject(parent)
	, m_ops(ops)
	, m_dev(dev)
{
	setObjectName(name());
}

QString IIOTrigger::name() const { return m_ops->name(m_dev); }
