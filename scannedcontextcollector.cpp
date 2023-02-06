#include "scannedcontextcollector.h"
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_SCANCTXCOLLECTOR, "ScanContextCollector")

ScannedContextCollector::ScannedContextCollector(QObject *parent)
	: QObject{parent}
{
	qDebug(CAT_SCANCTXCOLLECTOR) << "ctor";
}

ScannedContextCollector::~ScannedContextCollector()
{
	qDebug(CAT_SCANCTXCOLLECTOR) << "dtor";
}

void ScannedContextCollector::update(QStringList list)
{
	QSet<QString> updatedUris = QSet<QString>(list.begin(),list.end());
	auto newUris = updatedUris - uris;
	auto deletedUris = uris - updatedUris;

	qDebug(CAT_SCANCTXCOLLECTOR) << "cached uris:" << uris;
	for (const auto &uri : newUris) {
		qInfo(CAT_SCANCTXCOLLECTOR) << "new device found: " << uri;
		Q_EMIT newDevice(uri);

	}

	for (const auto &uri : deletedUris) {
		qInfo(CAT_SCANCTXCOLLECTOR) << "to delete device: " << uri;
		Q_EMIT deleteDevice(uri);

	}

	uris = updatedUris;


}

void ScannedContextCollector::clearCache()
{
	uris.clear();
}
