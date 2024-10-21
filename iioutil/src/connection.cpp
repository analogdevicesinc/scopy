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

#include "connection.h"

using namespace scopy;

Connection::Connection(QString uri)
{
	this->m_uri = uri;
	this->m_context = nullptr;
	this->m_commandQueue = nullptr;
	this->m_refCount = 0;
}

Connection::~Connection()
{
	if(this->m_commandQueue) {
		delete this->m_commandQueue;
		this->m_commandQueue = nullptr;
	}
	if(this->m_context) {
		iio_context_destroy(this->m_context);
		this->m_context = nullptr;
	}
}

const QString &Connection::uri() const { return m_uri; }

CommandQueue *Connection::commandQueue() const { return m_commandQueue; }

iio_context *Connection::context() const { return m_context; }

int Connection::refCount() const { return m_refCount; }

void Connection::open()
{
	if(!this->m_context) {
		this->m_context = iio_create_context_from_uri(this->m_uri.toStdString().c_str());
		if(this->m_context) {
			this->m_commandQueue = new CommandQueue();
			this->m_refCount++;
		}
	} else {
		this->m_refCount++;
	}
}

void Connection::closeAll()
{
	this->m_refCount = 0;
	close();
}

void Connection::close()
{
	this->m_refCount--;
	if(this->m_refCount <= 0) {
		/* If the open() and close() number of calls done by a client
		 * is mismatched, all the remaining clients should be notified of the
		 * destruction. */
		this->m_refCount = 0;
		Q_EMIT aboutToBeDestroyed();
	}
}

#include "moc_connection.cpp"
