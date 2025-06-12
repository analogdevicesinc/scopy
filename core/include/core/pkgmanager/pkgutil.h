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

#ifndef PKGUTIL_H
#define PKGUTIL_H

#include "../scopy-core_export.h"
#include <QObject>
#include <kzip.h>
#include <QFileInfoList>

#define METADATA_FILE "MANIFEST.json"
#define STATUS_BAR_MS 3000

namespace scopy {
class SCOPY_CORE_EXPORT PkgUtil
{
public:
	static bool removePkg(const QString &path);
	static bool extractZip(const QString &zipPath, const QString &destPath);
	static QJsonObject extractJsonMetadata(const QString &zipPath);
	static QJsonObject getMetadata(const QString &path);

	static bool validatePkg(QJsonObject &metadata);

	static QString checkPkgPath(const QString &path, const QString &pkgId);

	static QFileInfoList getFilesInfo(const QString &path, const QStringList &dirFilter,
					  const QStringList &fileFilter);

private:
	static QString validateArchiveEntry(const KZip &zip);
	static void addExtraField(QJsonObject &metadata, const QString &field, const QJsonValue &value);
	static void addFileMetadata(QJsonObject &metadata, const QString &path);
	static QString getFileSize(const QString &path);
};
} // namespace scopy

#endif // PKGUTIL_H
