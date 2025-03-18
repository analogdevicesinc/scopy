#include "pkgmanager.h"
#include "pkgutil.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <common/scopyconfig.h>
#include <QFileSystemModel>
#include <QLoggingCategory>
#include <QTreeView>

Q_LOGGING_CATEGORY(CAT_PKGMANAGER, "PkgManager")

using namespace scopy;

PkgManager *PkgManager::pinstance_{nullptr};
QMap<QString, QVariantMap> PkgManager::validPackages_{};
QString PkgManager::packagesPath_{};

PkgManager::PkgManager(QObject *parent)
	: QObject(parent)
{
	packagesPath_ = scopy::config::pkgFolderPath();
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

void PkgManager::init(const QStringList &paths) { pinstance_->_init(paths); }

void PkgManager::_init(const QStringList &paths)
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
		return false;
	}

	QString pkgName = metadata["name"].toString();
	QString pkgPath = PkgUtil::checkPkgPath(packagesPath_, pkgName);
	if(!pkgPath.isEmpty()) {
		Q_EMIT pkgExists(zipPath, pkgName);
		return false;
	}
	bool installed = PkgUtil::extractZip(zipPath, packagesPath_);
	if(installed) {
		Q_EMIT pkgInstalled(performRestart);
	} else {
		qWarning(CAT_PKGMANAGER) << "Couldn't install:" << metadata["name"];
	}

	return installed;
}

bool PkgManager::uninstall(const QString &pkgName, bool performRestart)
{
	return pinstance_->_uninstall(pkgName, performRestart);
}

bool PkgManager::_uninstall(const QString &pkgName, bool performRestart)
{
	QString pkgPath = PkgUtil::checkPkgPath(packagesPath_, pkgName);
	if(pkgPath.isEmpty()) {
		qWarning(CAT_PKGMANAGER) << "There is no package:" << pkgName;
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

void PkgManager::loadPkg(const QFileInfo &fileInfo)
{
	QString metadataPath = fileInfo.absoluteFilePath();
	QJsonObject metadata = PkgUtil::getMetadata(metadataPath);
	if(!PkgUtil::validatePkg(metadata)) {
		qWarning(CAT_PKGMANAGER) << "MANIFEST file validation failed:" << metadataPath;
		return;
	}
	validPackages_.insert(fileInfo.absolutePath(), metadata.toVariantMap());
	// Q_EMIT pkgLoaded(metadata.toVariantMap());
}

void PkgManager::createPkgDirectory()
{
	QDir pkgDir(packagesPath_);
	if(!pkgDir.exists()) {
		pkgDir.mkpath(packagesPath_);
	}
}

QFileInfoList PkgManager::listFilesInfo(const QStringList &filter)
{
	QFileInfoList files;
	const QStringList packagesPath = validPackages_.keys();
	for(const QString &pkgPath : packagesPath) {
		files.append(getFilesInfo(pkgPath, filter));
	}
	return files;
}

QStringList PkgManager::listFilesPath(const QStringList &filter)
{
	QStringList files;
	const QStringList packagesPath = validPackages_.keys();
	for(const QString &pkgPath : packagesPath) {
		files.append(getFilesPath(pkgPath, filter));
	}
	return files;
}

QList<QVariantMap> PkgManager::getPkgsMeta() { return validPackages_.values(); }

QStringList PkgManager::getInstalledPkgs()
{
	QStringList nameList;
	const QList<QVariantMap> pkgsMeta = validPackages_.values();
	std::transform(pkgsMeta.cbegin(), pkgsMeta.cend(), std::back_inserter(nameList),
		       [](const QVariantMap &v) { return v["name"].toString(); });
	return nameList;
}

QFileInfoList PkgManager::getFilesInfo(const QString &path, const QStringList &filterList)
{
	QDirIterator it(path, filterList, QDir::NoFilter, QDirIterator::Subdirectories);
	QFileInfoList files;
	while(it.hasNext()) {
		QDir dir(it.next());
		files.append(dir.entryInfoList(QDir::Files));
	}
	return files;
}

QStringList PkgManager::getFilesPath(const QString &path, const QStringList &filterList)
{
	QDirIterator it(path, filterList, QDir::NoFilter, QDirIterator::Subdirectories);
	QStringList files;
	while(it.hasNext()) {
		QDir dir(it.next());
		const QFileInfoList infoList = dir.entryInfoList(QDir::Files);
		for(const QFileInfo &info : infoList) {
			files.append(info.absoluteFilePath());
		}
	}
	return files;
}
