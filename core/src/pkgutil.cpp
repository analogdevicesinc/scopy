#include "pkgutil.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

#include <common/scopyconfig.h>

Q_LOGGING_CATEGORY(CAT_PKGUTIL, "PkgUtil")

using namespace scopy;

bool PkgUtil::copyDir(const QString &src, const QString &dst)
{
	QDir srcDir(src);
	if(!srcDir.exists()) {
		return false;
	}

	QDir destDir(dst);
	if(destDir.exists()) {
		destDir.removeRecursively();
	}
	if(!destDir.mkpath(dst)) {
		return false;
	}

	foreach(const QString &entry, srcDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
		QString srcPath = src + "/" + entry;
		QString destPath = dst + "/" + entry;
		QFileInfo srcInfo(srcPath);
		if(srcInfo.isDir()) {
			if(!copyDir(srcPath, destPath)) {
				return false;
			}
		} else {
			if(!QFile::copy(srcPath, destPath)) {
				return false;
			}
		}
	}
	return true;
}

bool PkgUtil::copyItem(const QString &src, const QString &dst)
{
	QFileInfo srcInfo(src);

	if(!srcInfo.exists()) {
		qWarning(CAT_PKGUTIL) << "Src does not exist:" << src;
		return false;
	}

	if(srcInfo.isFile()) {
		QFile dstFile(dst);
		if(dstFile.exists()) {
			dstFile.remove();
		}
		return QFile::copy(src, dst);
	} else if(srcInfo.isDir()) {
		return copyDir(src, dst);
	}
	return false;
}

QJsonObject PkgUtil::readLocalRepository(const QString &filePath)
{
	QFile file(filePath);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning(CAT_PKGUTIL) << "The local repo doesn't exist or cannot be oppened:" << filePath;
		return {};
	}

	QByteArray jsonData = file.readAll();
	file.close();

	QJsonDocument doc = QJsonDocument::fromJson(jsonData);
	if(doc.isNull()) {
		qDebug(CAT_PKGUTIL) << "JSON document is null:" << filePath;
	}

	return doc.object();
}

void PkgUtil::updateLocalRepository(const QString &filePath, const QJsonObject &jsonObject)
{
	QFile file(filePath);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qWarning(CAT_PKGUTIL) << "Cannot open the local repo for writing:" << filePath;
		return;
	}

	QJsonDocument doc(jsonObject);
	file.write(doc.toJson());
	file.close();
}

bool PkgUtil::removeFile(const QString &path)
{
	bool removed = false;
	QFileInfo fInfo(path);
	if(fInfo.isFile()) {
		removed = QFile::remove(path);
	}
	if(fInfo.isDir()) {
		QDir dir(path);
		removed = dir.removeRecursively();
	}
	if(!removed) {
		qWarning(CAT_PKGUTIL) << "Cannot remove file:" << path;
	}

	return removed;
}

QStringList PkgUtil::extractZip(const QString &zipPath, const QString &destPath)
{
	bool success = true;
	QStringList files;
	KZip zip(zipPath);
	if(!zip.open(QIODevice::ReadOnly)) {
		qWarning(CAT_PKGUTIL) << "Cannot open the zip file:" << zipPath;
		return files;
	}
	QStringList fileList;
	const KArchiveDirectory *dir = zip.directory();
	for(const QString &entry : dir->entries()) {
		if(!dir->entry(entry)->isDirectory()) {
			continue;
		}
		const KArchiveDirectory *dirToExtract = static_cast<const KArchiveDirectory *>(dir->entry(entry));
		if(!dirToExtract) {
			continue;
		}
		QString destDirPath = destPath + QDir::separator() + dirToExtract->name();
		getArchiveFiles(dirToExtract, fileList, destDirPath);
		if(fileList.isEmpty()) {
			qWarning(CAT_PKGUTIL) << "Don't overwrite existing files!";
			break;
		}
		success &= dirToExtract->copyTo(destDirPath);
	}
	if(!success) {
		fileList.clear();
	}
	return fileList;
}

QJsonObject PkgUtil::extractJsonMetadata(const QString &zipPath)
{
	KZip zip(zipPath);
	if(!zip.open(QIODevice::ReadOnly)) {
		qWarning(CAT_PKGUTIL) << "Cannot open the zip file:" << zipPath;
		return {};
	}
	const KArchiveFile *manifestFile = zip.directory()->file(DESCRIPTOR_FILE);
	if(!manifestFile) {
		qWarning(CAT_PKGUTIL) << DESCRIPTOR_FILE << "not found in package!";
		return {};
	}
	QJsonDocument doc = QJsonDocument::fromJson(manifestFile->data());
	if(!doc.isObject()) {
		qWarning(CAT_PKGUTIL) << "QJsonDocument parsing failed:" << DESCRIPTOR_FILE;
		return {};
	}
	return doc.object();
}

void PkgUtil::getArchiveFiles(const KArchiveDirectory *dir, QStringList &fileList, const QString &destPath,
			      const QString &prefix)
{
	// Iterate over all entries in the directory
	const QStringList entries = dir->entries();
	for(const QString &entryName : entries) {
		const KArchiveEntry *entry = dir->entry(entryName);
		QString entryPath = prefix + QDir::separator() + entryName;
		QString installPath = destPath + entryPath;

		if(entry->isFile()) {
			if(QFile::exists(installPath)) {
				fileList.clear();
				return;
			}
			fileList.append(installPath);
		} else if(entry->isDirectory()) {
			// If the directory doesn't exist save the path to that directory.
			// The entire directory will be removed upon uninstallation.
			if(!QFile::exists(installPath)) {
				fileList.append(installPath);
				continue;
			}
			const KArchiveDirectory *subDir = static_cast<const KArchiveDirectory *>(entry);
			getArchiveFiles(subDir, fileList, destPath, entryPath);
		}
	}
}

// Better in pkgInstaller ????
QStringList PkgUtil::getPkgsName()
{
	QJsonObject localRepository = readLocalRepository(scopy::config::localPluginFolderPath());
	return localRepository.keys();
}
