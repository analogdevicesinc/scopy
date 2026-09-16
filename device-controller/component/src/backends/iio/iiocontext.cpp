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

#include "component/backends/iio/iiocontext.h"

#include "iioutil/ibackend.h"
#include "iioutil/icontextops.h"

using namespace scopy::component::iio;

IIOContext::~IIOContext()
{
	if(!m_backend) {
		return;
	}

	const QObjectList kids = children();
	for(QObject *child : kids) {
		delete child;
		child = nullptr;
	}

	if(m_handle.ptr) {
		m_backend->contextOps()->destroyContext(m_handle);
	}
}
