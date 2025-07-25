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

#include "pkgutil.h"
#include "pkgmanifestfields.h"

#include <QDir>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <qjsonarray.h>
#include <common/loggingutil.h>
#include <common/scopyconfig.h>
#include <gui/plot_utils.hpp>

Q_LOGGING_CATEGORY(CAT_PKGUTIL, "PkgUtil")

using namespace scopy;

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
		LoggingUtil::logMessage(CAT_PKGUTIL, QString(METADATA_FILE) + " not found in package!",
					LoggingUtil::Warning, true, true, STATUS_BAR_MS);
		return {};
	}
	QJsonDocument doc = QJsonDocument::fromJson(manifestFile->data());
	if(!doc.isObject()) {
		LoggingUtil::logMessage(CAT_PKGUTIL, "QJsonDocument parsing failed: " + QString(METADATA_FILE),
					LoggingUtil::Warning, true, true, STATUS_BAR_MS);
		return {};
	}
	QJsonObject obj = doc.object();
	addFileMetadata(obj, zipPath);
	return obj;
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
	QJsonObject obj = doc.object();
	addFileMetadata(obj, QFileInfo(path).path());
	return obj;
}

// This method validates the package metadata by checking required fields.
// Additional checks could include verifying field formats or ensuring compatibility with specific versions.
bool PkgUtil::validatePkg(QJsonObject &metadata)
{
	if(metadata.isEmpty()) {
		qWarning(CAT_PKGUTIL) << "Empty metadata!";
		return false;
	}
	// Validate metadata fields
	for(const QString &field : PkgManifest::requiredFields) {
		QJsonValue value = metadata.value(field);
		// Check if field is missing or null
		if(value.isNull() || value.isUndefined()) {
			LoggingUtil::logMessage(CAT_PKGUTIL, "Missing required field: " + field, LoggingUtil::Warning,
						true, true, STATUS_BAR_MS);
			return false;
		}
		// Check if field is empty (string or array)
		if((value.isString() && value.toString().trimmed().isEmpty()) ||
		   (value.isArray() && value.toArray().isEmpty())) {
			LoggingUtil::logMessage(CAT_PKGUTIL, "Empty required field: " + field, LoggingUtil::Warning,
						true, true, STATUS_BAR_MS);
			return false;
		}
	}
	return true;
}

QString PkgUtil::checkPkgPath(const QString &path, const QString &pkgId)
{
	QDir pkgsDir(path);
	const QFileInfoList pkgs = pkgsDir.entryInfoList(QDir::Dirs);
	for(const QFileInfo &p : pkgs) {
		if(p.fileName().contains(pkgId)) {
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
		LoggingUtil::logMessage(CAT_PKGUTIL, "There must be a single entry in the archive! (a directory)",
					LoggingUtil::Warning, true, false, STATUS_BAR_MS);
		return "";
	}
	QString entryName = zipEntries.first();
	if(!zipDir->entry(entryName)->isDirectory()) {
		LoggingUtil::logMessage(CAT_PKGUTIL, "The archive entry must be a directory!", LoggingUtil::Warning,
					true, false, STATUS_BAR_MS);
		return "";
	}
	return entryName;
}

void PkgUtil::addExtraField(QJsonObject &metadata, const QString &field, const QJsonValue &value)
{
	metadata.insert(field, value);
}

void PkgUtil::addFileMetadata(QJsonObject &metadata, const QString &path)
{
	QFileInfo pkgInfo(path);
	QString size = getFileSize(path);
	QString lastModifiedDate = pkgInfo.lastModified().toString();
	addExtraField(metadata, "size", size);
	addExtraField(metadata, "last_modified", lastModifiedDate);
	addExtraField(metadata, PkgManifest::PKG_PATH, path);
}

QString PkgUtil::getFileSize(const QString &path)
{
	QString result;
	int fileSizeBytes = QFileInfo(path).size();
	MetricPrefixFormatter formatter;
	result = formatter.format(fileSizeBytes, "B", 2);

	return result;
}

QFileInfoList PkgUtil::getFilesInfo(const QString &path, const QStringList &dirFilter = {},
				    const QStringList &fileFilter = {})
{
	QFileInfoList files;
	QStringList nameFilters = (dirFilter.isEmpty()) ? fileFilter : dirFilter;
	QDirIterator it(path, nameFilters, QDir::NoFilter, QDirIterator::Subdirectories);
	while(it.hasNext()) {
		if(dirFilter.isEmpty()) {
			files.append(it.next());
		} else {
			QDir dir(it.next());
			files.append(dir.entryInfoList(fileFilter, QDir::Files));
		}
	}
	return files;
}
