#include "contextprovider.h"
#include <QLoggingCategory>
#include <QDebug>
#include <QApplication>
#include <QElapsedTimer>

Q_LOGGING_CATEGORY(CAT_CTXMGR, "ContextProvider")

using namespace adiscope;
ContextRefCounter::ContextRefCounter(QString uri)
{
	this->uri = uri;
	this->refcnt++;
	QElapsedTimer t;
	t.start();
	this->ctx = iio_create_context_from_uri(uri.toStdString().c_str());
	qInfo(CAT_CTXMGR) << "Acquiring context "<< uri <<"took "<<t.elapsed();
}

ContextRefCounter::~ContextRefCounter() {
	if(this->ctx) {
		iio_context_destroy(this->ctx);
	}
}


ContextProvider* ContextProvider::pinstance_{nullptr};
std::mutex ContextProvider::mutex_;

ContextProvider::ContextProvider(QObject *parent) : QObject(parent)
{
	qDebug(CAT_CTXMGR)<<"ctor";
}

ContextProvider::~ContextProvider()
{
	qDebug(CAT_CTXMGR)<<"dtor";
}

ContextProvider *ContextProvider::GetInstance()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (pinstance_ == nullptr)
	{
		pinstance_ = new ContextProvider(QApplication::instance()); // singleton has the app as parent
	} else {
		qDebug(CAT_CTXMGR)<<"got instance from singleton";
	}
	return pinstance_;
}

iio_context *ContextProvider::open(QString uri)
{
	std::lock_guard<std::mutex> lock(mutex_);
	iio_context *ctx = nullptr;
	if(map.contains(uri)) {
		map.value(uri)->refcnt++;
		qDebug(CAT_CTXMGR)<< uri << "opening - found - refcnt++ = " << map.value(uri)->refcnt;
	} else {
		ContextRefCounter* ctxref = new ContextRefCounter(uri);
		if(ctxref->ctx==nullptr) {
			qWarning(CAT_CTXMGR) << uri << "not a valid context";
			delete ctxref;
			return nullptr;
		}
		map.insert(uri, ctxref);
		qDebug(CAT_CTXMGR)<< uri << "opening  - created in map - refcnt = "<< map.value(uri)->refcnt;
	}

	ctx = map.value(uri)->ctx;
	return ctx;
}

void ContextProvider::close(QString uri)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if(map.contains(uri)) {
		map.value(uri)->refcnt--;
		qDebug(CAT_CTXMGR)<< uri << "closing - found in map - refcnt-- = " << map.value(uri)->refcnt;
		if(map[uri]->refcnt == 0) {
			delete map[uri];
			map.remove(uri);
			qDebug(CAT_CTXMGR)<< uri << "removed from map";
		}
	} else {
		qInfo(CAT_CTXMGR) << uri << "not found in map. nop";
	}
}

/*
auto cm = ContextManager::GetInstance();
ContextScanner *cs = new ContextScanner(this);
cs->startScan(5000);
cm->open("ip:192.168.2.1");
cm->open("ip:192.168.2.1");
cm->open("ip:192.168.2.1");
cm->close("ip:192.168.2.1");
auto cd = ContextManager::GetInstance();
cd->close("ip:192.168.2.1");
cd->close("ip:192.168.2.1");*/



#include "moc_contextprovider.cpp"
