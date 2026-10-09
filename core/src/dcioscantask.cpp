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

#include "dcioscantask.h"

#include "component/backends/iio/iiocontextfactory.h"
#include "iioutil/ibackend.h"
#include "iioutil/iioscan.h"

#include <QLoggingCategory>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_DCIIOSCANCTX, "DcIioScanTask");

DcIioScanTask::DcIioScanTask(QObject *parent)
	: QThread(parent)
{}

DcIioScanTask::~DcIioScanTask() {}

void DcIioScanTask::run()
{
	QVector<QPair<QString, QString>> ctxs;
	int ret = scan(&ctxs, scanParams);
	if(ret >= 0 && !isInterruptionRequested()) {
		Q_EMIT scanFinished(ctxs);
	}
}

void DcIioScanTask::setScanParams(QString s) { scanParams = s; }

int DcIioScanTask::scan(QVector<QPair<QString, QString>> *ctxs, QString scanParams)
{
	scopy::iio::IBackend *backend = scopy::component::iio::IIOContextFactory::defaultBackend();
	if(!backend) {
		qWarning(CAT_DCIIOSCANCTX) << "no libiio backend";
		return -1;
	}
	scopy::iio::IIOScan scanner(backend->scanOps());
	const QVector<scopy::iio::ScanResult> results = scanner.scan(scanParams);
	for(const scopy::iio::ScanResult &r : results) {
		ctxs->append({parseDescription(r.description), r.uri});
	}
	qDebug(CAT_DCIIOSCANCTX) << "scanned" << *ctxs;
	return ctxs->size();
}

QString DcIioScanTask::parseDescription(const QString &d)
{
	int startPos = d.indexOf('(');
	if(startPos == -1) {
		return d;
	}
	int endPos = d.indexOf(")), serial=");
	if(endPos == -1) {
		endPos = d.lastIndexOf(')');
	} else {
		startPos += 1;
	}
	return d.mid(startPos, endPos - startPos + 1);
}

#include "moc_dcioscantask.cpp"
