/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "dongledevicemanager.h"

#include "devicemanager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

#include <pkg-manager/pkgmanager.h>

Q_LOGGING_CATEGORY(CAT_DONGLEDEVICEMANAGER, "DongleDeviceManager")

using namespace scopy;

namespace {
// The simulated dongle: {"uri": "dongle:sim0", "name": "dongleDevice"}
QJsonObject readSimulatedIdentity()
{
	const QFileInfoList files = PkgManager::listFilesInfo({"eprom_test"}, {"dongle.json"});
	if(files.isEmpty()) {
		return {};
	}
	QFile file(files.first().absoluteFilePath());
	if(!file.open(QIODevice::ReadOnly)) {
		qWarning(CAT_DONGLEDEVICEMANAGER) << "Can't read" << file.fileName();
		return {};
	}
	QJsonParseError err;
	const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
	if(!doc.isObject()) {
		qWarning(CAT_DONGLEDEVICEMANAGER) << "Invalid identity file" << file.fileName() << err.errorString();
		return {};
	}
	return doc.object();
}
} // namespace

DongleDeviceManager::DongleDeviceManager(DeviceManager *dm, QObject *parent)
	: QObject(parent)
	, m_dm(dm)
{}

void DongleDeviceManager::addSimulatedDevices()
{
	const QJsonObject identity = readSimulatedIdentity();
	const QString uri = identity.value("uri").toString();
	if(uri.isEmpty()) {
		qDebug(CAT_DONGLEDEVICEMANAGER) << "eprom_test package not installed, no dongleDevice";
		return;
	}
	qInfo(CAT_DONGLEDEVICEMANAGER) << "Adding dongleDevice" << uri << identity.value("name").toString();
	m_dm->createDevice(CATEGORY, uri);
}

#include "moc_dongledevicemanager.cpp"
