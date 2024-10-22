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

ScannedIIOContextCollector::~ScannedIIOContextCollector() { qDebug(CAT_SCANCTXCOLLECTOR) << "dtor"; }

void ScannedIIOContextCollector::update(QVector<QPair<QString, QString>> ctxsDescription)
{
	// Do we need to map Device* to uri in this class ?
	QSet<QString> updatedUris;
	for(const auto &pair : ctxsDescription) {
		// pair.first = description, pair.second = uri
		updatedUris.insert(pair.second);
	}

	updatedUris = updatedUris + lockedUris;

	auto newUris = updatedUris - uris;
	auto deletedUris = uris - updatedUris;

	qDebug(CAT_SCANCTXCOLLECTOR) << "cached uris:" << uris;
	for(const auto &uri : newUris) {
		qInfo(CAT_SCANCTXCOLLECTOR) << "new device found: " << uri;
		//		if(!lockedUris.contains(uri))
		Q_EMIT foundDevice("iio", uri);
	}

	for(const auto &uri : deletedUris) {
		qInfo(CAT_SCANCTXCOLLECTOR) << "to delete device: " << uri;
		//		if(!lockedUris.contains(uri))
		Q_EMIT lostDevice("iio", uri);
	}
	uris = updatedUris;
}

void ScannedIIOContextCollector::removeDevice(QString id, Device *d) { uris.remove(d->param()); }

void ScannedIIOContextCollector::lock(QString uri, Device *d)
{

	if(uris.contains(d->param()))
		lockedUris.insert(d->param());
}

void ScannedIIOContextCollector::unlock(QString uri, Device *d)
{
	if(uris.contains(d->param()))
		lockedUris.remove(d->param());
}

void ScannedIIOContextCollector::clearCache() { uris.clear(); }

#include "moc_scannediiocontextcollector.cpp"
