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

#include "pkgmanager.h"
#include "pkgmanifestfields.h"
#include "pkgutil.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <common/loggingutil.h>
#include <common/scopyconfig.h>
#include <QFileSystemModel>
#include <QLoggingCategory>
#include <QTreeView>
#include <pluginbase/preferences.h>

Q_LOGGING_CATEGORY(CAT_PKGMANAGER, "PkgManager")

using namespace scopy;

PkgManager *PkgManager::pinstance_{nullptr};
QMap<QString, QVariantMap> PkgManager::validPackages_{};
QString PkgManager::packagesPath_{};

PkgManager::PkgManager(QObject *parent)
	: QObject(parent)
{
	packagesPath_ = Preferences::get("packages_path").toString();
	createPkgDirectory();
}

PkgManager::~PkgManager() {}

PkgManager *PkgManager::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new PkgManager(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

void PkgManager::init(QSet<QString> paths)
{
	paths.insert(packagesPath_);
	pinstance_->_init(paths);
}

void PkgManager::_init(const QSet<QString> &paths)
{
	for(const QString &p : paths) {
		QDirIterator it(p, {"MANIFEST.json"}, QDir::Files, QDirIterator::Subdirectories);
		while(it.hasNext()) {
			QFileInfo file(it.next());
			loadPkg(file);
		}
	}
}

bool PkgManager::install(const QString &zipPath, bool performRestart)
{
	return pinstance_->_install(zipPath, performRestart);
}

bool PkgManager::_install(const QString &zipPath, bool performRestart)
{
	if(packagesPath_.isEmpty()) {
		return false;
	}
	QJsonObject metadata = PkgUtil::extractJsonMetadata(zipPath);
	if(!PkgUtil::validatePkg(metadata)) {
		LoggingUtil::logMessage(CAT_PKGMANAGER,
					"Could not install the package. The file appears to be invalid or corrupted.",
					LoggingUtil::Warning, true, false, STATUS_BAR_MS);
		return false;
	}

	QString pkgName = metadata[PkgManifest::PKG_ID].toString();
	QString pkgPath = PkgUtil::checkPkgPath(packagesPath_, pkgName);
	if(!pkgPath.isEmpty()) {
		Q_EMIT pkgExists(zipPath, pkgName);
		return false;
	}
	bool installed = PkgUtil::extractZip(zipPath, packagesPath_);
	if(installed) {
		Q_EMIT pkgInstalled(performRestart);
	} else {
		LoggingUtil::logMessage(CAT_PKGMANAGER, "Couldn't install: " + metadata[PkgManifest::PKG_ID].toString(),
					LoggingUtil::Warning, true, false, STATUS_BAR_MS);
	}

	return installed;
}

bool PkgManager::uninstall(const QString &pkgId, bool performRestart)
{
	return pinstance_->_uninstall(pkgId, performRestart);
}

bool PkgManager::_uninstall(const QString &pkgId, bool performRestart)
{
	QString pkgPath = validPackages_[pkgId].value(PkgManifest::PKG_PATH).toString();
	if(pkgPath.isEmpty()) {
		LoggingUtil::logMessage(CAT_PKGMANAGER, "Couldn't find the path for package " + pkgId,
					LoggingUtil::Warning, true, false, STATUS_BAR_MS);
		return false;
	}
	bool removed = PkgUtil::removePkg(pkgPath);
	if(removed) {
		Q_EMIT pkgUninstalled(performRestart);
	}
	return removed;
}

bool PkgManager::preview(const QString &zipPath) { return pinstance_->_preview(zipPath); }

bool PkgManager::_preview(const QString &zipPath)
{
	QJsonObject metadata = PkgUtil::extractJsonMetadata(zipPath);
	bool valid = PkgUtil::validatePkg(metadata);
	if(valid) {
		Q_EMIT zipMetadata(metadata.toVariantMap());
	}
	return valid;
}

QFileInfo PkgManager::reverseSearch(const QString &filePath)
{
	for(auto it = validPackages_.begin(); it != validPackages_.end(); ++it) {
		const QString pkgPath = it.value()[PkgManifest::PKG_PATH].toString();
		if(filePath.contains(pkgPath)) {
			return pkgPath;
		}
	}
	qWarning(CAT_PKGMANAGER) << "Couldn't find the package of the file:" << filePath;
	return QFileInfo();
}

void PkgManager::loadPkg(const QFileInfo &fileInfo)
{
	QString metadataPath = fileInfo.absoluteFilePath();
	QJsonObject metadata = PkgUtil::getMetadata(metadataPath);
	if(!PkgUtil::validatePkg(metadata)) {
		qWarning(CAT_PKGMANAGER) << "MANIFEST file validation failed:" << metadataPath;
		return;
	}
	validPackages_.insert(metadata[PkgManifest::PKG_ID].toString(), metadata.toVariantMap());
}

void PkgManager::createPkgDirectory()
{
	QDir pkgDir(packagesPath_);
	if(!pkgDir.exists()) {
		pkgDir.mkpath(packagesPath_);
	}
}

QString PkgManager::packagesPath() { return packagesPath_; }

QFileInfoList PkgManager::listFilesInfo(QStringList dirFilter, QStringList fileFilter)
{
	QFileInfoList files;
	for(auto &it : validPackages_) {
		QString pkgPath = it[PkgManifest::PKG_PATH].toString();
		files.append(PkgUtil::getFilesInfo(pkgPath, dirFilter, fileFilter));
	}
	return files;
}

QList<QVariantMap> PkgManager::getPkgsMeta() { return validPackages_.values(); }

QStringList PkgManager::getPkgsName()
{
	QStringList nameList;
	const QList<QVariantMap> pkgsMeta = validPackages_.values();
	std::transform(pkgsMeta.cbegin(), pkgsMeta.cend(), std::back_inserter(nameList),
		       [](const QVariantMap &v) { return v[PkgManifest::PKG_TITLE].toString(); });
	return nameList;
}

QStringList PkgManager::getPkgsAuthor()
{
	QStringList authorList;
	const QList<QVariantMap> pkgsMeta = validPackages_.values();
	std::transform(pkgsMeta.cbegin(), pkgsMeta.cend(), std::back_inserter(authorList),
		       [](const QVariantMap &v) { return v[PkgManifest::PKG_AUTHOR].toString(); });
	return authorList;
}

QStringList PkgManager::getPkgsVersion()
{
	QStringList versList;
	const QList<QVariantMap> pkgsMeta = validPackages_.values();
	std::transform(pkgsMeta.cbegin(), pkgsMeta.cend(), std::back_inserter(versList),
		       [](const QVariantMap &v) { return v[PkgManifest::PKG_VERSION].toString(); });
	return versList;
}

QStringList PkgManager::getPkgsCategory()
{
	QStringList categoryList;
	const QList<QVariantMap> pkgsMeta = validPackages_.values();
	for(const QVariantMap &pkg : pkgsMeta) {
		QStringList pkgCat = pkg[PkgManifest::PKG_CATEGORY].toStringList();
		categoryList.append(pkgCat);
	}
	return categoryList;
}

#include "moc_pkgmanager.cpp"
