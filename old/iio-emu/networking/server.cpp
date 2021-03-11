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


#include "server.hpp"
#include "iiod/factory_ops.hpp"

#define PORT 30431

using namespace iio_emu;

Server::Server(QObject *parent, const char *type)
	: QObject(parent)
{
	m_server = std::unique_ptr<QTcpServer>(new QTcpServer());

	connect(m_server.get(), SIGNAL(newConnection()),
		this, SLOT(slotNewConnection()));

	if(!m_server->listen(QHostAddress::Any, PORT))
	{
		qDebug()<< "Server could not be started";
		return;
	}
	else
	{
		qDebug()<< "Server started";
	}

	FactoryOps factory;
	m_ops = factory.getOps(type);
	if (m_ops == nullptr) {
		qDebug()<< "No such device";
		return;
	}
	m_iiod = tinyiiod_create(m_ops->getIIODOps());
}

Server::~Server()
{
	if (m_iiod) {
		tinyiiod_destroy(m_iiod);
	}
	delete m_ops;
}

void Server::slotNewConnection()
{
	qDebug()<<"Connected";
	QTcpSocket* s = m_server->nextPendingConnection();
	Socket* socket = new Socket(s);

	connect(socket, SIGNAL(dataReady(Socket*)),this, SLOT(receiveData(Socket*)));
	connect(socket, SIGNAL(socketDisconnected(Socket*)),this, SLOT(disconnectSocket(Socket*)));
}

void Server::receiveData(Socket *socket)
{
	qDebug()<<"Receive data";

	m_ops->setCurrentSocket(socket);

	if (m_iiod) {
		tinyiiod_read_command(m_iiod);
	}
}

void Server::disconnectSocket(Socket *socket)
{
	//delete socket;
	qDebug()<< "Disconnected";
}
