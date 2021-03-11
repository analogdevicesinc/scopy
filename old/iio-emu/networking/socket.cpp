/*
 * Copyright (c) 2021 Analog Devices Inc.
 *
 * This file is part of iio-emu
 * (see http://www.github.com/analogdevicesinc/iio-emu).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "socket.hpp"

using namespace iio_emu;

Socket::Socket(QTcpSocket *socket)
{
	this->socket = std::unique_ptr<QTcpSocket>(socket);

	connect (socket, SIGNAL (readyRead()), this, SLOT (slotReadyRead()));
	connect (socket, SIGNAL (disconnected()), this, SLOT (slotDisconnected()));
	connect (socket, SIGNAL (connected()), this, SLOT (slotConnected()));
}

void Socket::slotReadyRead ()
{
	Q_EMIT dataReady (this);
}

void Socket::slotConnected()
{
	Q_EMIT socketConnected(this);
}

void Socket::slotDisconnected()
{
	Q_EMIT socketDisconnected(this);
}

QByteArray Socket::getData()
{
	QByteArray data = socket->readLine();
	if (!data.size()) {
		socket->waitForReadyRead();
		data = socket->readLine();
	}
	return data;
}

QByteArray Socket::getData(size_t len)
{
	QByteArray data = socket->read(len);
	if (!data.size()) {
		socket->waitForReadyRead();
		data = socket->read(len);
	}
	return data;
}

void Socket::write(const char *buf, size_t len)
{
	socket->write(buf, len);
}
