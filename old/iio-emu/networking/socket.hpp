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

#ifndef IIO_EMU_SOCKET_HPP
#define IIO_EMU_SOCKET_HPP

#include <QObject>
#include <QTcpSocket>
#include <memory>

namespace iio_emu {

class Socket : public QObject {
	Q_OBJECT
public:
	explicit Socket(QTcpSocket *socket);
	~Socket() = default;

	QByteArray getData();
	QByteArray getData(size_t len);
	void write(const char *buf, size_t len);

public:
	std::unique_ptr<QTcpSocket> socket;

private Q_SLOTS:
	void slotReadyRead();
	void slotConnected();
	void slotDisconnected();

Q_SIGNALS:
	void dataReady(Socket *socket);
	void socketConnected(Socket *socket);
	void socketDisconnected(Socket *socket);
};
}
#endif //IIO_EMU_SOCKET_HPP
