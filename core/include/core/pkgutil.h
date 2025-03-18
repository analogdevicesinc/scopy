#ifndef PKGUTIL_H
#define PKGUTIL_H

#include <scopy-core_export.h>
#include <QObject>
#include <kzip.h>

#define METADATA_FILE "MANIFEST.json"

namespace scopy {
class SCOPY_CORE_EXPORT PkgUtil
{
public:
	static bool removePkg(const QString &path);
	static bool extractZip(const QString &zipPath, const QString &destPath);
	static QJsonObject extractJsonMetadata(const QString &zipPath);
	static QJsonObject getMetadata(const QString &path);

	static bool validatePkg(QJsonObject &metadata);

	static QString checkPkgPath(const QString &path, const QString &pkgName);

private:
	static QString validateArchiveEntry(const KZip &zip);

	static const QStringList REQUIRED_FIELDS;
};
} // namespace scopy

#endif // PKGUTIL_H
