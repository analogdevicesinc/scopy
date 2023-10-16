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

Connection *ConnectionProvider::open(QString uri) { return ConnectionProvider::GetInstance()->_open(uri); }

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

void ConnectionProvider::closeAll(QString uri) { return ConnectionProvider::GetInstance()->_closeAll(uri); }

void ConnectionProvider::close(QString uri) { return ConnectionProvider::GetInstance()->_close(uri); }

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
