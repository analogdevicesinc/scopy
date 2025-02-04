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

#include <iioutil/connectionprovider.h>
#include <iioutil/iiocpp/iioresult.h>
#include <iioutil/iiocpp/iioattribute.h>
#include <iioutil/iiocpp/iiocontext.h>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_IIO_DEVICEIMPL, "IIODevice")

using namespace scopy;
void IIODeviceImpl::init()
{
	auto cp = ConnectionProvider::GetInstance();

	// Optimization for iio plugins - keep context open while running compatible

	cp->open(m_param);
	DeviceImpl::init();
	cp->close(m_param);
}

bool IIODeviceImpl::verify()
{
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(!conn) {
		return false;
	}
	ConnectionProvider::GetInstance()->close(m_param);
	return true;
}

QMap<QString, QString> IIODeviceImpl::readDeviceInfo()
{
	QMap<QString, QString> contextAttributes;
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
	if(!conn) {
		qWarning(CAT_IIO_DEVICEIMPL) << "Cannot read the device info! (unavailable context)";
	} else {
		for(int i = 0; i < IIOContext::get_attrs_count(conn->context()); i++) {
			IIOResult<const iio_attr *> attrRes = IIOContext::get_attr(conn->context(), i);
			if(!attrRes.ok()) {
				qWarning(CAT_IIO_DEVICEIMPL) << "Cannot read the device info! (unavailable attribute)";
				continue;
			}

			const iio_attr *attr = attrRes.data();
			const char *name = IIOAttribute::get_name(attr);
			const char *value = IIOAttribute::get_static_value(attr); // FIXME: This should work
			contextAttributes[name] = value;
		}
		ConnectionProvider::GetInstance()->close(m_param);
	}

	return contextAttributes;
}
