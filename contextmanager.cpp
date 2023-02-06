#include "contextmanager.h"
#include <QLoggingCategory>
#include <QDebug>
#include <QApplication>

Q_LOGGING_CATEGORY(CAT_CTXMGR, "ContextManager")

ContextRefCounter::ContextRefCounter(QString uri)
{
	this->uri = uri;
	this->refcnt++;
	this->ctx = iio_create_context_from_uri(uri.toStdString().c_str());
}

ContextRefCounter::~ContextRefCounter() {
	iio_context_destroy(this->ctx);
}


ContextManager* ContextManager::pinstance_{nullptr};
std::mutex ContextManager::mutex_;

ContextManager::ContextManager(QObject *parent) : QObject(parent)
{
	qDebug(CAT_CTXMGR)<<"ctor";
}

ContextManager::~ContextManager()
{
	qDebug(CAT_CTXMGR)<<"dtor";
}

ContextManager *ContextManager::GetInstance()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (pinstance_ == nullptr)
	{
		pinstance_ = new ContextManager(QApplication::instance()); // singleton has the app as parent
	} else {
		qDebug(CAT_CTXMGR)<<"got instance from singleton";
	}
	return pinstance_;
}

iio_context *ContextManager::open(QString uri)
{
	std::lock_guard<std::mutex> lock(mutex_);
	iio_context *ctx;	
	if(map.contains(uri)) {
		map.value(uri)->refcnt++;
		qDebug(CAT_CTXMGR)<< uri << "opening - found - refcnt++ = " << map.value(uri)->refcnt;
	} else {
		map.insert(uri, new ContextRefCounter(uri));
		qDebug(CAT_CTXMGR)<< uri << "opening  - created in map - refcnt = "<< map.value(uri)->refcnt;
	}

	ctx = map.value(uri)->ctx;
	return ctx;
}

void ContextManager::close(QString uri)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if(map.contains(uri)) {
		map.value(uri)->refcnt--;
		qDebug(CAT_CTXMGR)<< uri << "closing - found in map - refcnt-- = " << map.value(uri)->refcnt;
		if(map[uri]->refcnt == 0) {
			map.remove(uri);
			qDebug(CAT_CTXMGR)<< uri << "removed from map";
		}
	}
}


