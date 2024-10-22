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

#include "connectionprovider.h"
#include <QLoggingCategory>
#include <QApplication>

Q_LOGGING_CATEGORY(CAT_CONNECTIONMGR, "ConnectionProvider")

using namespace scopy;

ConnectionProvider *ConnectionProvider::pinstance_{nullptr};
std::mutex ConnectionProvider::mutex_;

scopy::ConnectionProvider::ConnectionProvider(QObject *parent)
	: QObject(parent)
{
	qDebug(CAT_CONNECTIONMGR) << "ConnectionProvider object ctor";
}

scopy::ConnectionProvider::~ConnectionProvider() { qDebug(CAT_CONNECTIONMGR) << "ConnectionProvider object dtor"; }

ConnectionProvider *ConnectionProvider::GetInstance()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if(pinstance_ == nullptr) {
		pinstance_ = new ConnectionProvider(QApplication::instance()); // singleton has the app as parent
	} else {
		qDebug(CAT_CONNECTIONMGR) << "ConnectionProvider: Got instance from singleton";
	}
	return pinstance_;
}

Connection *ConnectionProvider::open(struct iio_context *ctx) { return ConnectionProvider::GetInstance()->_open(ctx); }

Connection *ConnectionProvider::open(QString uri) { return ConnectionProvider::GetInstance()->_open(uri); }

Connection *ConnectionProvider::_open(struct iio_context *ctx)
{
	std::unique_lock<std::mutex> lock(mutex_);
	for(auto it = map.keyValueBegin(); it != map.keyValueEnd(); ++it) {
		if(ctx == it->second->context()) {
			lock.unlock();
			return _open(it->first);
		}
	}
	return nullptr;
}

Connection *ConnectionProvider::_open(QString uri)
{
	std::lock_guard<std::mutex> lock(mutex_);
	Connection *connectionObject = nullptr;
	if(!map.contains(uri)) {
		connectionObject = new Connection(uri);
		map.insert(uri, connectionObject);
		qDebug(CAT_CONNECTIONMGR) << uri << " created in map ";
	} else {
		connectionObject = map.value(uri);
		qDebug(CAT_CONNECTIONMGR) << uri << " found in map ";
	}
	connectionObject->open();
	if(connectionObject->refCount() == 0) {
		qDebug(CAT_CONNECTIONMGR) << uri << " Connection: failed to open, removed from map.";
		map.remove(uri);
		delete connectionObject;
		return nullptr;
	}
	qDebug(CAT_CONNECTIONMGR) << uri << " Connection: open, refcount++ = " << connectionObject->refCount();
	return connectionObject;
}

void ConnectionProvider::closeAll(struct iio_context *ctx) { return ConnectionProvider::GetInstance()->_closeAll(ctx); }

void ConnectionProvider::close(struct iio_context *ctx) { return ConnectionProvider::GetInstance()->_close(ctx); }

void ConnectionProvider::closeAll(Connection *conn) { return ConnectionProvider::GetInstance()->_closeAll(conn); }

void ConnectionProvider::close(Connection *conn) { return ConnectionProvider::GetInstance()->_close(conn); }

void ConnectionProvider::closeAll(QString uri) { return ConnectionProvider::GetInstance()->_closeAll(uri); }

void ConnectionProvider::close(QString uri) { return ConnectionProvider::GetInstance()->_close(uri); }

void ConnectionProvider::_closeAll(struct iio_context *ctx)
{
	std::unique_lock<std::mutex> lock(mutex_);
	for(auto it = map.keyValueBegin(); it != map.keyValueEnd(); ++it) {
		if(ctx == it->second->context()) {
			lock.unlock();
			_closeAll(it->first);
			break;
		}
	}
}

void ConnectionProvider::_close(struct iio_context *ctx)
{
	std::unique_lock<std::mutex> lock(mutex_);
	for(auto it = map.keyValueBegin(); it != map.keyValueEnd(); ++it) {
		if(ctx == it->second->context()) {
			lock.unlock();
			_close(it->first);
			break;
		}
	}
}

void ConnectionProvider::_closeAll(Connection *conn)
{
	std::unique_lock<std::mutex> lock(mutex_);
	for(auto it = map.keyValueBegin(); it != map.keyValueEnd(); ++it) {
		if(conn == it->second) {
			lock.unlock();
			_closeAll(it->first);
			break;
		}
	}
}

void ConnectionProvider::_close(Connection *conn)
{
	std::unique_lock<std::mutex> lock(mutex_);
	for(auto it = map.keyValueBegin(); it != map.keyValueEnd(); ++it) {
		if(conn == it->second) {
			lock.unlock();
			_close(it->first);
			break;
		}
	}
}

void ConnectionProvider::_closeAll(QString uri)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if(map.contains(uri)) {
		map.value(uri)->closeAll();
	}
	_closeAndRemove(uri);
}

void ConnectionProvider::_close(QString uri)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if(map.contains(uri)) {
		map.value(uri)->close();
	}
	_closeAndRemove(uri);
}

void ConnectionProvider::_closeAndRemove(QString uri)
{
	if(map.contains(uri)) {
		qDebug(CAT_CONNECTIONMGR)
			<< uri << " Connection: closing - found in map - refcnt-- = " << map.value(uri)->refCount();
		if(map.value(uri)->refCount() == 0) {
			delete map[uri];
			map.remove(uri);
			qDebug(CAT_CONNECTIONMGR) << uri << " Connection: destroyed, removed from map";
		}
	} else {
		qInfo(CAT_CONNECTIONMGR) << uri << " Connection: not found in map. nop";
	}
}

#include "moc_connectionprovider.cpp"
