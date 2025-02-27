#ifndef PKGUTIL_H
#define PKGUTIL_H

#include <scopy-core_export.h>
#include <QObject>
#include <kzip.h>

#define DESCRIPTOR_FILE "MANIFEST.json"
namespace scopy {
class SCOPY_CORE_EXPORT PkgUtil
{
public:
	static bool copyDir(const QString &source, const QString &destination);
	static bool copyItem(const QString &src, const QString &dst);
	static QJsonObject readLocalRepository(const QString &filePath);
	static void updateLocalRepository(const QString &filePath, const QJsonObject &jsonObject);
	static bool removeFile(const QString &path);
	static QStringList extractZip(const QString &zipPath, const QString &destPath);
	static QJsonObject extractJsonMetadata(const QString &zipPath);
	static QStringList getPkgsName();

private:
	static void getArchiveFiles(const KArchiveDirectory *dir, QStringList &fileList, const QString &destPath,
				    const QString &prefix = "");
};
} // namespace scopy

#endif // PKGUTIL_H
