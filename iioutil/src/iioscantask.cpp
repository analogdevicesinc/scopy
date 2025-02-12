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

#include <iio/iio.h>
#include "iiocpp/iioresult.h"
#include "iiocpp/iioscan.h"

#include <QElapsedTimer>
#include <QLoggingCategory>

#include <libserialport.h>

using namespace scopy;
IIOScanTask::IIOScanTask(QObject *parent)
	: QThread(parent)
{
}

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
	qDebug(CAT_IIOSCANCTX) << "start scanning";
	IIOResult<struct iio_scan *> scan_res;
	struct iio_scan *scan = nullptr;
	iio_context_params *params = nullptr;
	int num_contexts;
	int i;

	QElapsedTimer et;
	et.start();
	if(scanParams.isEmpty()) {
		scan_res = IIOScan::scan(params, nullptr);
	} else {
		scan_res = IIOScan::scan(params, scanParams.toStdString().c_str());
	}

	if(!scan_res.ok()) {
		qWarning(CAT_IIOSCANCTX) << "scan error - " << scan_res.error();
		return -1;
	}
	scan = scan_res.data();
	num_contexts = IIOScan::get_results_count(scan);

	qDebug(CAT_IIOSCANCTX) << "found " << num_contexts << "contexts in " << et.elapsed() << "miliseconds ";
	for(i = 0; i < num_contexts; i++) {
		QString description = parseDescription(QString(IIOScan::get_description(scan, i)));
		ctxs->append({description, QString(IIOScan::get_uri(scan, i))});
	}
	qDebug(CAT_IIOSCANCTX) << "scanned " << *ctxs;

scan_err:
	IIOScan::destroy(scan);
	return num_contexts;
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
	sp_port **serialPorts;
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
