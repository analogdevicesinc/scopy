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

#ifndef PKGMANAGER_H
#define PKGMANAGER_H

#include <QFileInfoList>
#include <QMap>
#include <QObject>

namespace scopy {
class PkgManager : public QObject
{
	Q_OBJECT
protected:
	explicit PkgManager(QObject *parent = nullptr);
	~PkgManager();

public:
	// singleton
	PkgManager(PkgManager &other) = delete;
	void operator=(const PkgManager &) = delete;
	static PkgManager *GetInstance();
	static void init();
	void _init(const QStringList &paths);

	static bool install(const QString &zipPath = "", bool performRestart = true);
	bool _install(const QString &zipPath = "", bool performRestart = true);
	static bool uninstall(const QString &pkgId, bool performRestart = true);
	bool _uninstall(const QString &pkgId = "", bool performRestart = true);
	static bool update(const QString &pkgName);

	static bool preview(const QString &zipPath = "");
	bool _preview(const QString &zipPath = "");

	static QFileInfo reverseSearch(const QString &filePath);

	static QFileInfoList listFilesInfo(QStringList dirFilter = {}, QStringList fileFilter = {});

	static QStringList getPkgsName();
	static QStringList getPkgsAuthor();
	static QStringList getPkgsVersion();
	static QStringList getPkgsCategory();
	static QList<QVariantMap> getPkgsMeta();

Q_SIGNALS:
	void pkgExists(const QString &zipPath, const QString &pkgName);

	void pkgInstalled(bool restart);
	void pkgUninstalled(bool restart);

	void zipMetadata(QVariantMap metadata);

private:
	void loadPkg(const QFileInfo &fileInfo);
	void createPkgDirectory();

	static QString packagesPath_;
	static PkgManager *pinstance_;
	static QMap<QString, QVariantMap> validPackages_;
};
} // namespace scopy

#endif // PKGMANAGER_H
