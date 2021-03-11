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

#include "network_ops.hpp"

ssize_t iio_emu::socket_read_line(Socket *socket, char *buf, size_t len)
{
	QByteArray dataReceived = socket->getData();
	memcpy(buf, dataReceived.data(), dataReceived.size());

	uint32_t i;
	bool found = false;

	for (i = 0; i < len; i++) {
		if (buf[i] == '\n')
			found = true;
		else if (found)
			break;
	}
	if (!found) {
		/* No \n found -> garbage data */
		return -EIO;
	}

	i--;
	buf[i - 1] = '\0';

	return len;
}

ssize_t iio_emu::socket_read(Socket *socket, void *buf, size_t len)
{
	QByteArray dataReceived = socket->getData(len);
	qDebug() << "DATA: " << dataReceived;
	memcpy(buf, dataReceived.data(), dataReceived.size());

	return dataReceived.size();
}
