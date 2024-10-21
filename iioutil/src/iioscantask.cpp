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

#include "iioscantask.h"

#include <iio.h>

#include <QElapsedTimer>
#include <QLoggingCategory>

#include <libserialport.h>

using namespace scopy;
IIOScanTask::IIOScanTask(QObject *parent)
	: QThread(parent)
{}

IIOScanTask::~IIOScanTask() {}

Q_LOGGING_CATEGORY(CAT_IIOSCANCTX, "IIOScanTask");

void IIOScanTask::run()
{
	QVector<QPair<QString, QString>> ctxs;
	int ret = IIOScanTask::scan(&ctxs, scanParams);
	if(isInterruptionRequested())
		return;
	if(ret >= 0)
		Q_EMIT scanFinished(ctxs);
}

void IIOScanTask::setScanParams(QString s) { scanParams = s; }

int IIOScanTask::scan(QVector<QPair<QString, QString>> *ctxs, QString scanParams)
{
	qInfo(CAT_IIOSCANCTX) << "start scanning";
	struct iio_scan_context *scan_ctx = NULL;
	struct iio_context_info **info;
	int num_contexts;
	int ret;
	int i;

	QElapsedTimer et;
	et.start();
	if(scanParams.isEmpty()) {
		scan_ctx = iio_create_scan_context(NULL, 0);
	} else {
		scan_ctx = iio_create_scan_context(scanParams.toStdString().c_str(), 0);
	}

	if(!scan_ctx) {
		qWarning(CAT_IIOSCANCTX) << "no scan context - " << errno << " " << strerror(errno);
		return -1;
	}
	ret = iio_scan_context_get_info_list(scan_ctx, &info);
	if(ret < 0) {
		qWarning(CAT_IIOSCANCTX) << "iio_scan_context_get_info_list error - " << errno << " "
					 << strerror(errno);
		goto scan_err;
	}

	num_contexts = ret;

	qDebug(CAT_IIOSCANCTX) << "found " << num_contexts << "contexts in " << et.elapsed() << "miliseconds ";
	for(i = 0; i < num_contexts; i++) {
		QString description = parseDescription(QString(iio_context_info_get_description(info[i])));
		ctxs->append({description, QString(iio_context_info_get_uri(info[i]))});
	}
	iio_context_info_list_free(info);
	qDebug(CAT_IIOSCANCTX) << "scanned " << *ctxs;

scan_err:
	iio_scan_context_destroy(scan_ctx);
	return ret;
}

QString IIOScanTask::parseDescription(const QString &d)
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

	QString description = d.mid(startPos, endPos - startPos + 1);
	return description;
}

QVector<QString> IIOScanTask::getSerialPortsName()
{
	QVector<QString> serialPortsName;
	struct sp_port **serialPorts;
	int retCode = sp_list_ports(&serialPorts);
	if(retCode == SP_OK) {
		for(int i = 0; serialPorts[i]; i++) {
			serialPortsName.push_back(QString(sp_get_port_name(serialPorts[i])));
		}
		sp_free_port_list(serialPorts);
	}
	return serialPortsName;
}

#include "moc_iioscantask.cpp"
