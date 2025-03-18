#include "pkgutil.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <qjsonarray.h>

#include <common/scopyconfig.h>

Q_LOGGING_CATEGORY(CAT_PKGUTIL, "PkgUtil")

using namespace scopy;

const QStringList PkgUtil::REQUIRED_FIELDS{"name", "version", "license", "author", "scopy_compatibility", "category"};

bool PkgUtil::removePkg(const QString &path)
{
	bool removed = false;
	QFileInfo fInfo(path);
	if(fInfo.isDir()) {
		QDir dir(path);
		removed = dir.removeRecursively();
	}
	if(!removed) {
		qWarning(CAT_PKGUTIL) << "Cannot remove file:" << path;
	}

	return removed;
}

bool PkgUtil::extractZip(const QString &zipPath, const QString &destPath)
{
	KZip zip(zipPath);
	if(!zip.open(QIODevice::ReadOnly)) {
		qWarning(CAT_PKGUTIL) << "Cannot open the zip file:" << zipPath;
		return false;
	}
	QString entryName = validateArchiveEntry(zip);
	if(entryName.isEmpty()) {
		return false;
	}
	QString extractedPath = destPath + QDir::separator() + entryName;
	QDir d(extractedPath);
	if(d.exists()) {
		qWarning(CAT_PKGUTIL) << "A package with an identical name was discovered at:" << destPath;
		return false;
	}
	const KArchiveDirectory *dir = zip.directory();
	return dir->copyTo(destPath);
}

QJsonObject PkgUtil::extractJsonMetadata(const QString &zipPath)
{
	KZip zip(zipPath);
	if(!zip.open(QIODevice::ReadOnly)) {
		qWarning(CAT_PKGUTIL) << "Cannot open the zip file:" << zipPath;
		return {};
	}
	QString entryName = validateArchiveEntry(zip);
	if(entryName.isEmpty()) {
		return {};
	}
	const KArchiveDirectory *zipDir = static_cast<const KArchiveDirectory *>(zip.directory()->entry(entryName));
	const KArchiveFile *manifestFile = zipDir->file(METADATA_FILE);
	if(!manifestFile) {
		qWarning(CAT_PKGUTIL) << METADATA_FILE << "not found in package!";
		return {};
	}
	QJsonDocument doc = QJsonDocument::fromJson(manifestFile->data());
	if(!doc.isObject()) {
		qWarning(CAT_PKGUTIL) << "QJsonDocument parsing failed:" << METADATA_FILE;
		return {};
	}
	return doc.object();
}

QJsonObject PkgUtil::getMetadata(const QString &path)
{
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly)) {
		qWarning(CAT_PKGUTIL) << "Cannot open the MANIFEST file:" << path;
		return {};
	}
	QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
	if(!doc.isObject()) {
		qWarning(CAT_PKGUTIL) << "QJsonDocument parsing failed:" << METADATA_FILE;
		return {};
	}
	return doc.object();
}

// In this method we also can make another checks
bool PkgUtil::validatePkg(QJsonObject &metadata)
{
	// Validate metadata fields
	for(const QString &field : REQUIRED_FIELDS) {
		QJsonValue value = metadata.value(field);
		// Check if field is missing or null
		if(value.isNull() || value.isUndefined()) {
			qWarning(CAT_PKGUTIL) << "Missing required field:" << field;
			return false;
		}
		// Check if field is empty (string or array)
		if((value.isString() && value.toString().trimmed().isEmpty()) ||
		   (value.isArray() && value.toArray().isEmpty())) {
			qWarning(CAT_PKGUTIL) << "Empty required field:" << field;
			return false;
		}
	}
	return true;
}

QString PkgUtil::checkPkgPath(const QString &path, const QString &pkgName)
{
	QDir pkgsDir(path);
	const QFileInfoList pkgs = pkgsDir.entryInfoList(QDir::Dirs);
	for(const QFileInfo &p : pkgs) {
		if(p.fileName().contains(pkgName)) {
			return p.absoluteFilePath();
		}
	}
	return "";
}

QString PkgUtil::validateArchiveEntry(const KZip &zip)
{
	const KArchiveDirectory *zipDir = zip.directory();
	if(!zipDir) {
		return "";
	}
	QStringList zipEntries = zipDir->entries();
	if(zipEntries.size() != 1) {
		qWarning(CAT_PKGUTIL) << "There must be a single entry in the archive!";
		return "";
	}
	QString entryName = zipEntries.first();
	if(!zipDir->entry(entryName)->isDirectory()) {
		qWarning(CAT_PKGUTIL) << "The archive entry must be a directory!!";
		return "";
	}
	return entryName;
}
