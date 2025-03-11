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
	qDebug(CAT_IIOSCANCTX) << "start scanning";
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

QMap<QString, QString> IIOScanTask::getSerialPortsName()
{
	QMap<QString, QString> serialPortsName;
	struct sp_port **serialPorts;
	int retCode = sp_list_ports(&serialPorts);
	if(retCode == SP_OK) {
		for(int i = 0; serialPorts[i]; i++) {
			QString fullDescription = "";
			QString fullName = "";

			char *p_name = sp_get_port_name(serialPorts[i]);
			if(p_name) {
				fullName += "[ ";
				fullName += QString(p_name);
				fullName += " ]";
			}

			char *p_description = sp_get_port_description(serialPorts[i]);
			if(p_description) {
				fullName += " ( ";
				fullName += QString(p_description);
				fullName += " )";

				fullDescription += QString(p_description);
				fullDescription += "\n";
			}

			int usb_bus, usb_addr;
			retCode = sp_get_port_usb_bus_address(serialPorts[i], &usb_bus, &usb_addr);
			if(retCode == SP_OK) {
				fullDescription += "USB bus: ";
				fullDescription += QString::number(usb_bus);
				fullDescription += " USB address: ";
				fullDescription += QString::number(usb_addr);
				fullDescription += "\n";
			}

			int usb_vid, usb_pid;
			retCode = sp_get_port_usb_vid_pid(serialPorts[i], &usb_vid, &usb_pid);
			if(retCode == SP_OK) {
				fullDescription += "USB VID: ";
				fullDescription += QString::number(usb_vid, 16);
				fullDescription += " USB PID: ";
				fullDescription += QString::number(usb_pid, 16);
				fullDescription += "\n";
			}

			char *p_manufacturer = sp_get_port_usb_manufacturer(serialPorts[i]);
			if(p_manufacturer) {
				fullDescription += "Manufacturer: ";
				fullDescription += QString(p_manufacturer);
				fullDescription += "\n";
			}

			char *p_usbProd = sp_get_port_usb_product(serialPorts[i]);
			if(p_usbProd) {
				fullDescription += "USB Product: ";
				fullDescription += QString(p_usbProd);
				fullDescription += "\n";
			}

			char *p_usbSerial = sp_get_port_usb_serial(serialPorts[i]);
			if(p_usbSerial) {
				fullDescription += "USB Serial: ";
				fullDescription += QString(p_usbSerial);
				fullDescription += "\n";
			}

			char *p_btAddr = sp_get_port_bluetooth_address(serialPorts[i]);
			if(p_btAddr) {
				fullDescription += "Bluetooth MAC address: ";
				fullDescription += QString(p_btAddr);
				fullDescription += "\n";
			}
			serialPortsName.insert(fullName, fullDescription);
		}
		sp_free_port_list(serialPorts);
	}
	return serialPortsName;
}

#include "moc_iioscantask.cpp"
