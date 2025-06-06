/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef SCOPYMAINWINDOW_API_H
#define SCOPYMAINWINDOW_API_H

#include "scopy-core_export.h"
#include "scopymainwindow.h"
#include "scanbuttoncontroller.h"
#include "iioutil/connectionprovider.h"
#include <QFile>

namespace scopy {
class SCOPY_CORE_EXPORT ScopyMainWindow_API : public ApiObject
{
	Q_OBJECT
public:
	explicit ScopyMainWindow_API(ScopyMainWindow *w);
	~ScopyMainWindow_API();

	Q_INVOKABLE void acceptLicense();
	Q_INVOKABLE QString addDevice(QString uri, QString cat = "iio", bool async = false);
	Q_INVOKABLE QString addDevice(QString uri, QList<QString> plugins, QString cat = "iio", bool async = false);
	Q_INVOKABLE bool removeDevice(QString uri, QString cat = "iio");
	Q_INVOKABLE bool removeDevice(int idx);
	Q_INVOKABLE bool startScan(bool scanState);
	Q_INVOKABLE QStringList getDevicesName();
	Q_INVOKABLE bool connectDevice(int idx);
	Q_INVOKABLE bool connectDevice(QString devID);
	Q_INVOKABLE bool disconnectDevice(QString devID);
	Q_INVOKABLE bool disconnectDevice();
	Q_INVOKABLE bool switchTool(QString devID, QString toolName);
	Q_INVOKABLE bool switchTool(QString toolName);
	Q_INVOKABLE void runScript(QString scriptPath, bool exitApp = true);
	Q_INVOKABLE void runScriptList(QStringList scriptPathList, bool exitApp = true);
	Q_INVOKABLE void exit();
	Q_INVOKABLE QStringList getTools();
	Q_INVOKABLE QStringList getToolsForPlugin(QString plugin);
	Q_INVOKABLE QPair<QString, QVariant> getPreference(QString prfName);
	Q_INVOKABLE QMap<QString, QVariant> getPreferences();
	Q_INVOKABLE void setPreference(QString preName, QVariant value);
	Q_INVOKABLE void aboutPage();
	Q_INVOKABLE QStringList getPlugins(int idx);
	Q_INVOKABLE QStringList getPlugins(QString param, QString cat = "iio");
	Q_INVOKABLE bool getToolBtnState(QString tool);
	Q_INVOKABLE bool runTool(QString tool, bool flag);
	Q_INVOKABLE bool loadSetup(QString fileName, QString path = QCoreApplication::applicationDirPath());
	Q_INVOKABLE bool saveSetup(QString fileName, QString path = QCoreApplication::applicationDirPath());

	Q_INVOKABLE bool install(QString zipPath);
	Q_INVOKABLE bool extractZip(QString zipPath, QString dest);
	Q_INVOKABLE QVariantMap extractMetadata(QString zipPath);
	Q_INVOKABLE bool uninstall(const QString &pkgName);

	Q_INVOKABLE QStringList listFiles(const QStringList &dirFilter, const QStringList &fileFilter);
	Q_INVOKABLE QString findPkgName(const QString &filePath);

private:
	static bool sortByUUID(const QString &k1, const QString &k2);
	const QString getScriptContent(QFile *file);
	QStringList availablePlugins(QString param, QString cat, Device *dev);
	Device *getDevice(int idx);
	ScopyMainWindow *m_w;
};

} // namespace scopy

#endif // SCOPYMAINWINDOW_API_H
