#include "pkginstaller.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <qdebug.h>
#include <qjsonobject.h>
#include <common/scopyconfig.h>
#include <QApplication>
#include <QLoggingCategory>
#include <application_restarter.h>
#include <qelapsedtimer.h>
#include "pkgutil.h"

Q_LOGGING_CATEGORY(CAT_PKGINSTALLER, "PkgInstaller")

using namespace scopy;

PkgInstaller *PkgInstaller::pinstance_{nullptr};
PkgInstaller::PkgInstaller(QObject *parent)
	: QObject(parent)
{
	QFile file(scopy::config::pkgLocalRepo());
	if(!file.exists()) {
		file.open(QIODevice::ReadWrite);
	}
	connect(this, &PkgInstaller::pkgMetadata, this, &PkgInstaller::onPkgMetadata);
}

PkgInstaller *PkgInstaller::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new PkgInstaller(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

void PkgInstaller::init() { PkgInstaller::GetInstance(); }

PkgInstaller::~PkgInstaller() {}

bool PkgInstaller::install(const QString &zipPath, bool performRestart)
{
	return pinstance_->_install(zipPath, performRestart);
}

bool PkgInstaller::_install(const QString &zipPath, bool performRestart)
{
	bool installed = false;
	QString execPath = scopy::config::executableFolderPath();
	if(execPath.isEmpty()) {
		return false;
	}
	QJsonObject metadata = PkgUtil::extractJsonMetadata(zipPath);
	if(!validatePkg(metadata)) {
		return false;
	}
	QString pkgName = metadata["name"].toString();
	QJsonObject localRepository = PkgUtil::readLocalRepository(scopy::config::pkgLocalRepo());
	if(localRepository.contains(pkgName)) {
		Q_EMIT pkgExists(pkgName, zipPath);
	} else {
		installed = processPkgInstalation(zipPath, execPath, metadata, localRepository, performRestart);
	}
	return installed;
}

bool PkgInstaller::uninstall(const QString &pkgName, bool performRestart)
{
	return pinstance_->_uninstall(pkgName, performRestart);
}

bool PkgInstaller::_uninstall(const QString &pkgName, bool performRestart)
{
	bool success = true;
	QVariantList files;
	QJsonObject localRepository = PkgUtil::readLocalRepository(scopy::config::pkgLocalRepo());
	if(!localRepository.contains(pkgName)) {
		qWarning(CAT_PKGINSTALLER) << pkgName << " cannot be found!";
		return false;
	}
	QJsonObject obj = localRepository[pkgName].toObject();
	if(obj.contains("files")) {
		files = obj["files"].toArray().toVariantList();
	}
	for(const QVariant &f : qAsConst(files)) {
		QString path(f.toString());
		success &= PkgUtil::removeFile(path);
	}

	if(success) {
		localRepository.remove(pkgName);
		PkgUtil::updateLocalRepository(scopy::config::pkgLocalRepo(), localRepository);
		Q_EMIT pkgUninstalled(performRestart);
	} else {
		qWarning(CAT_PKGINSTALLER) << "There may be residual files!";
	}

	return success;
}

bool PkgInstaller::update(const QString &pkgName) { return true; }

bool PkgInstaller::preview(const QString &zipPath) { return pinstance_->_preview(zipPath); }

bool PkgInstaller::_preview(const QString &zipPath)
{
	QString execPath = scopy::config::executableFolderPath();
	if(execPath.isEmpty()) {
		return false;
	}
	QJsonObject metadata = PkgUtil::extractJsonMetadata(zipPath);
	bool valid = validatePkg(metadata);
	if(valid) {
		Q_EMIT zipMetadata(metadata.toVariantMap());
	}

	return valid;
}

QStringList PkgInstaller::installedPkg() const { return {}; }

bool PkgInstaller::extractFromStore(const QString &pluginName, const QString &destPath) { return true; }

// In this method we also can make another checks
bool PkgInstaller::validatePkg(QJsonObject &metadata)
{
	// Validate metadata fields
	for(const QString &field : m_requiredFields) {
		QJsonValue value = metadata.value(field);
		// Check if field is missing or null
		if(value.isNull() || value.isUndefined()) {
			qWarning(CAT_PKGINSTALLER) << "Missing required field:" << field;
			return false;
		}
		// Check if field is empty (string or array)
		if((value.isString() && value.toString().trimmed().isEmpty()) ||
		   (value.isArray() && value.toArray().isEmpty())) {
			qWarning(CAT_PKGINSTALLER) << "Empty required field:" << field;
			return false;
		}
	}
	return true;
}

bool PkgInstaller::processPkgInstalation(const QString &zipPath, const QString &execPath, QJsonObject &metadata,
					 QJsonObject &localRepository, bool restart)
{
	QStringList installedFiles = PkgUtil::extractZip(zipPath, execPath);
	bool installed = !installedFiles.isEmpty();
	if(installed) {
		metadata.insert("files", QJsonValue(QJsonArray::fromStringList(installedFiles)));
		localRepository.insert(metadata["name"].toString(), metadata);
		PkgUtil::updateLocalRepository(scopy::config::pkgLocalRepo(), localRepository);
		Q_EMIT pkgInstalled(restart);
	} else {
		qWarning(CAT_PKGINSTALLER) << "Couldn't install:" << metadata["name"];
	}
	return installed;
}

// To be deleted
void PkgInstaller::onPkgMetadata(QVariantMap metadata)
{
	QMapIterator<QString, QVariant> it(metadata);
	while(it.hasNext()) {
		it.next();
		qInfo(CAT_PKGINSTALLER) << it.key() << ": " << it.value();
	}
}
