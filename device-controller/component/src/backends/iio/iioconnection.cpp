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

#include "component/backends/iio/iioconnection.h"

#include "component/backends/iio/iioping.h"
#include "component/backends/iio/iioprovider.h"
#include "iioutil/ibackend.h"
#include "iioutil/icontextops.h"

using namespace scopy::component::iio;

IIOConnection::IIOConnection(QString uri, scopy::iio::IBackend *backend, scopy::iio::ContextHandle handle,
			     QObject *parent)
	: Connection(std::move(uri), IIOProvider::KIND, parent)
	, m_backend(backend)
	, m_handle(handle)
{
	auto *ops = m_backend->contextOps();
	m_name = ops->name(m_handle);
	m_description = ops->description(m_handle);
	const scopy::iio::ContextVersion v = ops->getVersion(m_handle);
	m_version = {v.major, v.minor, v.gitTag};
}

IIOConnection::~IIOConnection()
{
	// Leaves still alive reference the handle: drop them before the context.
	const QObjectList kids = children();
	for(QObject *child : kids) {
		delete child;
	}
	if(m_handle.ptr) {
		m_backend->contextOps()->destroyContext(m_handle);
	}
}

void IIOConnection::start()
{
	auto *ping = new IIOPing(m_backend->contextOps(), m_handle, executor(), this);
	connect(ping, &Ping::connectionLost, this, &Connection::lost);
	ping->startMonitoring(2000);
}
