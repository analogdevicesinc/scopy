#include "scannediiocontextcollector.h"
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_SCANCTXCOLLECTOR, "ScannedIIOContextCollector")

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
	QSet<QString> updatedUris = QSet<QString>(list.begin(),list.end());
	auto newUris = updatedUris - uris;
	auto deletedUris = uris - updatedUris;

	qDebug(CAT_SCANCTXCOLLECTOR) << "cached uris:" << uris;
	for (const auto &uri : newUris) {
		qInfo(CAT_SCANCTXCOLLECTOR) << "new device found: " << uri;
		Q_EMIT foundDevice(uri);

	}

	for (const auto &uri : deletedUris) {
		qInfo(CAT_SCANCTXCOLLECTOR) << "to delete device: " << uri;
		Q_EMIT lostDevice(uri);

	}

	uris = updatedUris;


}

void ScannedIIOContextCollector::clearCache()
{
	uris.clear();
}
