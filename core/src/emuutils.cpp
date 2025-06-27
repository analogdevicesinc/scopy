/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "emuutils.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <pkg-manager/pkgmanager.h>
#include <common/scopyconfig.h>

using namespace scopy;

QJsonArray *EmuUtils::emuOptions_{nullptr};

QStringList EmuUtils::availableDevices()
{
	if(emuOptions_ == nullptr) {
		emuOptions_ = new QJsonArray(load());
	}
	QStringList availableDevices;
	for(const auto &object : qAsConst(*emuOptions_)) {
		QString device = object.toObject().value(QString("device")).toString();
		availableDevices.append(device);
	}
	return availableDevices;
}

QJsonArray EmuUtils::emuOptions()
{
	if(emuOptions_ == nullptr) {
		emuOptions_ = new QJsonArray(load());
	}
	return *emuOptions_;
}

void EmuUtils::cleanup()
{
	if(emuOptions_) {
		delete emuOptions_;
		emuOptions_ = nullptr;
	}
}

QJsonArray EmuUtils::load()
{
	QJsonArray emuOptions;
	QFileInfoList fileList = PkgManager::listFilesInfo(QStringList() << scopy::config::pkgEmuDir(),
							   QStringList() << scopy::config::emuSetupFile());
	for(QFileInfo &f : fileList) {
		QFile configFile(f.filePath());
		if(!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qWarning() << "Failed to open file:" << f.filePath();
			continue;
		}
		QJsonDocument d = QJsonDocument::fromJson(configFile.readAll());
		if(!d.isArray()) {
			qWarning() << "Invalid JSON format in file:" << f.filePath();
			continue;
		}
		QJsonArray entries = d.array();
		for(auto optConfig : entries) {
			QJsonObject jsonObj = optConfig.toObject();
			if(jsonObj.isEmpty()) {
				continue;
			}
			jsonObj.insert("path", f.absolutePath());
			emuOptions.append(jsonObj);
		}
	}
	return emuOptions;
}
