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

#include "scopyconfig.h"

#include "scopy-common_config.h"

#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QSysInfo>
#include <QUuid>

QString scopy::config::tempLogFilePath() { return QDir::cleanPath(settingsFolderPath() + "/" + SCOPY_TEMP_LOG_FILE); }

QString scopy::config::defaultTranslationFolderPath()
{
#if defined __APPLE__
	return QCoreApplication::applicationDirPath() + "/translations";
#elif defined(__appimage__)
	return QCoreApplication::applicationDirPath() + "/../lib/scopy/translations";
#endif
	return SCOPY_TRANSLATION_INSTALL_PATH;
}

QString scopy::config::localTranslationFolderPath() { return SCOPY_TRANSLATION_BUILD_PATH; }

QString scopy::config::defaultStyleFolderPath()
{
#if defined __APPLE__
	return QCoreApplication::applicationDirPath() + "/style";
#elif defined(__appimage__)
	return QCoreApplication::applicationDirPath() + "/../lib/scopy/style";
#endif
	return SCOPY_STYLE_INSTALL_PATH;
}

QString scopy::config::localStyleFolderPath() { return SCOPY_STYLE_BUILD_PATH; }

QString scopy::config::preferencesFolderPath()
{
	return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString scopy::config::settingsFolderPath()
{
	return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString scopy::config::executableFolderPath() { return QCoreApplication::applicationDirPath(); }

QString scopy::config::version() { return SCOPY_VERSION; }

QString scopy::config::gitCommit() { return SCOPY_VERSION_GIT; }

QString scopy::config::fullversion()
{
	QString ver = QString("v") + SCOPY_VERSION + "-g" + SCOPY_VERSION_GIT;
	return ver;
}

QString scopy::config::os() { return QSysInfo::prettyProductName(); }

QString scopy::config::pcSpecs()
{
	QString ret;
	ret.append("build_abi: " + QSysInfo::buildAbi());
	ret.append("\n");
	ret.append("build_cpu: " + QSysInfo::buildCpuArchitecture());
	ret.append("\n");
	ret.append("host: " + QSysInfo::machineHostName());
	ret.append("\n");
	ret.append("arch: " + QSysInfo::currentCpuArchitecture());
	ret.append("\n");
	ret.append("kernel: " + QSysInfo::kernelType());
	ret.append("\n");
	ret.append("kernel-ver: " + QSysInfo::kernelVersion());
	ret.append("\n");

	return ret;
}

QString scopy::config::pkgFolderPath()
{
	QString defaultPath =
		QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() + "packages";
#ifdef WIN32
	defaultPath = defaultPkgFolderPath();
#endif
	return defaultPath;
}

QString scopy::config::dump()
{
	QString ret;

	ret = QString("ScopyConfig");
	ret += QString("\n") + "DefaultPackageFolderPath: " + defaultPkgFolderPath();
	ret += QString("\n") + "PreferencesPluginFolderPath: " + preferencesFolderPath();
	ret += QString("\n") + "SettingsFolderPath: " + settingsFolderPath();
	ret += QString("\n") + "ExecutableFolderPath: " + executableFolderPath();
	ret += QString("\n") + "version: " + version();
	ret += QString("\n") + "gitCommit: " + gitCommit();
	ret += QString("\n") + "fullversion: " + fullversion();
	ret += QString("\n") + "os: " + os();
	ret += QString("\n") + "pcSpecs: \n" + pcSpecs();

	return ret;
}

QString scopy::config::getUuid()
{
#ifdef USE_QUUID
	return QUuid.createUuid().toString();
#else
	return QString::number(uuid++);
#endif
}

QString scopy::config::emuSetupFile() { return PKG_EMU_SETUP_FILE; }

QString scopy::config::pkgEmuDir() { return PKG_EMU_DIR; }

QString scopy::config::pkgPluginsDir() { return PKG_PLUGINS_DIR; }

QString scopy::config::pkgStyleThemesDir() { return PKG_STYLE_THEMES_DIR; }

QString scopy::config::pkgStylePropDir() { return PKG_STYLE_PROP_DIR; }

QString scopy::config::pkgTranslationsDir() { return PKG_TRANSLATION_DIR; }

QString scopy::config::localPkgFolderPath() { return SCOPY_PACKAGE_BUILD_PATH; }

QString scopy::config::defaultPkgFolderPath()
{

#ifdef WIN32
	// Scopy_install_folder/packages
	return QCoreApplication::applicationDirPath() + "/packages";
#elif defined __APPLE__
	// Scopy.app/Contents/MacOS/packages
	return QCoreApplication::applicationDirPath() + "/packages";
#elif defined(__appimage__)
	// usr/lib/packages
	return QCoreApplication::applicationDirPath() + "/../lib/scopy/packages";
#endif

	return SCOPY_PACKAGE_INSTALL_PATH;
}
