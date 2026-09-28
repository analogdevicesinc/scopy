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

#include "ad9361regmapplugin.h"
#include "fileregisterstrategy.h"
#include "scopy-ad9361regmap_config.h"

#include <QDir>
#include <QFile>
#include <QLoggingCategory>

#include <common/scopyconfig.h>
#include <component/controller.h>
#include <component/context.h>
#include <component/backends/iio/iiodevice.h>
#include <core/deviceimpl.h>
#include <pkg-manager/pkgmanager.h>
#include <pluginbase/statusbarmanager.h>
#include <regmap/regmapplugin.h>

Q_LOGGING_CATEGORY(CAT_AD9361REGMAP, "Ad9361RegmapPlugin")
using namespace scopy::ad9361regmap;

static const QString DEVICE_NAME = "ad9361-phy";
static const QString PKG_RESOURCE_DIR = "ad9361-regmap-demo";
static const QString XML_FILE = "ad9361-phy-demo.xml";
static const QString VALUES_FILE = "ad9361-phy-values.csv";

bool Ad9361RegmapPlugin::compatible(QString m_param, QString category)
{
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		return false;
	}
	return ctx->findChild<component::iio::IIODevice *>(DEVICE_NAME, Qt::FindDirectChildrenOnly) != nullptr;
}

bool Ad9361RegmapPlugin::loadPage() { return false; }

bool Ad9361RegmapPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

QString Ad9361RegmapPlugin::description() { return AD9361REGMAP_PLUGIN_DESCRIPTION; }

QString Ad9361RegmapPlugin::findPackageFile(const QString &fileName) const
{
	const QFileInfoList files = PkgManager::listFilesInfo({PKG_RESOURCE_DIR}, {fileName});
	return files.isEmpty() ? QString() : files.first().absoluteFilePath();
}

// The installed package may be read only, work on a copy in the Scopy settings folder
QString Ad9361RegmapPlugin::prepareValuesFile() const
{
	QString workDir = scopy::config::settingsFolderPath() + "/" + PKG_RESOURCE_DIR;
	QString workFile = workDir + "/" + VALUES_FILE;
	if(QFile::exists(workFile)) {
		return workFile;
	}

	QString pkgFile = findPackageFile(VALUES_FILE);
	if(pkgFile.isEmpty() || !QDir().mkpath(workDir) || !QFile::copy(pkgFile, workFile)) {
		qWarning(CAT_AD9361REGMAP) << "Can't create values file" << workFile << "from" << pkgFile;
		return QString();
	}
	QFile::setPermissions(workFile, QFile::permissions(workFile) | QFileDevice::WriteOwner);
	return workFile;
}

bool Ad9361RegmapPlugin::onConnect()
{
	if(!m_device) {
		return false;
	}

	auto *regmapPlugin = dynamic_cast<regmap::RegmapPlugin *>(m_device->getPluginByName("RegmapPlugin"));
	if(!regmapPlugin || !regmapPlugin->hasDevice(DEVICE_NAME)) {
		qWarning(CAT_AD9361REGMAP) << "Register Map plugin not available for" << DEVICE_NAME;
		StatusBarManager::pushMessage("AD9361 Regmap Demo requires the Register Map plugin to be enabled.",
					      5000);
		return false;
	}

	QString xmlPath = findPackageFile(XML_FILE);
	if(!xmlPath.isEmpty()) {
		regmapPlugin->setDeviceXml(DEVICE_NAME, xmlPath);
	}

	QString valuesPath = prepareValuesFile();
	if(valuesPath.isEmpty()) {
		StatusBarManager::pushMessage("AD9361 Regmap Demo: values file not found, using device access.", 5000);
		return true;
	}

	auto file = QSharedPointer<RegisterValuesFile>::create(valuesPath);
	regmapPlugin->setDeviceAccess(DEVICE_NAME, new FileRegisterReadStrategy(file),
				      new FileRegisterWriteStrategy(file));

	qInfo(CAT_AD9361REGMAP) << DEVICE_NAME << "register map uses" << xmlPath << "and values file" << valuesPath;
	StatusBarManager::pushMessage("AD9361 Regmap Demo: register values are read from " + valuesPath, 5000);
	return true;
}

bool Ad9361RegmapPlugin::onDisconnect() { return true; }

void Ad9361RegmapPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":2,
	   "category":[
	      "iio"
	   ]
	}
)plugin");
}
