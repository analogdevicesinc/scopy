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

#include "iiodeviceimpl.h"

#include <QLoggingCategory>

#include <component/attribute.h>

Q_LOGGING_CATEGORY(CAT_IIO_DEVICEIMPL, "IIODevice")

using namespace scopy;

void IIODeviceImpl::init() { DeviceImpl::init(); }

bool IIODeviceImpl::verify() { return static_cast<bool>(m_context); }

QMap<QString, QString> IIODeviceImpl::readDeviceInfo()
{
	QMap<QString, QString> contextAttributes;
	if(!m_context) {
		qWarning(CAT_IIO_DEVICEIMPL) << "Cannot read the device info! (unavailable context)";
	} else {
		const QList<component::Attribute *> attributes =
			m_context->findChildren<component::Attribute *>(Qt::FindDirectChildrenOnly);
		for(component::Attribute *attr : attributes) {
			if(!attr->readCapability()) {
				continue;
			}
			QCoro::waitFor(attr->readCapability()->readAsync());
			contextAttributes[attr->name()] = attr->cachedValue();
		}
	}

	return contextAttributes;
}
