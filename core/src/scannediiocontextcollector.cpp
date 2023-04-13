#include "scannediiocontextcollector.h"
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_SCANCTXCOLLECTOR, "ScannedIIOContextCollector")

using namespace scopy;
ScannedIIOContextCollector::ScannedIIOContextCollector(QObject *parent)
	: QObject{parent}
{
	qDebug(CAT_SCANCTXCOLLECTOR) << "ctor";
}

ScannedIIOContextCollector::~ScannedIIOContextCollector()
{
	qDebug(CAT_SCANCTXCOLLECTOR) << "dtor";
}

void ScannedIIOContextCollector::update(QStringList list)
{
	// Do we need to map Device* to uri in this class ?

	QSet<QString> updatedUris = QSet<QString>(list.begin(),list.end());
	updatedUris = updatedUris + lockedUris;

	auto newUris = updatedUris - uris;
	auto deletedUris = uris - updatedUris;

	qDebug(CAT_SCANCTXCOLLECTOR) << "cached uris:" << uris;
	for (const auto &uri : newUris) {
		qInfo(CAT_SCANCTXCOLLECTOR) << "new device found: " << uri;
//		if(!lockedUris.contains(uri))
			Q_EMIT foundDevice("iio",uri);

	}

	for (const auto &uri : deletedUris) {
		qInfo(CAT_SCANCTXCOLLECTOR) << "to delete device: " << uri;
//		if(!lockedUris.contains(uri))
			Q_EMIT lostDevice("iio",uri);

	}
	uris = updatedUris;
}

void ScannedIIOContextCollector::removeDevice(QString id, Device*d) {
	uris.remove(d->param());
}

void ScannedIIOContextCollector::lock(QString uri, Device* d) {

	if(uris.contains(d->param()))
		lockedUris.insert(d->param());
}

void ScannedIIOContextCollector::unlock(QString uri, Device *d) {
	if(uris.contains(d->param()))
		lockedUris.remove(d->param());
}

void ScannedIIOContextCollector::clearCache()
{
	uris.clear();
}

#include "moc_scannediiocontextcollector.cpp"
