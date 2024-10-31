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

#ifndef SCOPYMAINWINDOW_H
#define SCOPYMAINWINDOW_H

#include <QMainWindow>
#include <QOpenGLWidget>

#include "licenseoverlay.h"
#include "scanbuttoncontroller.h"
#include "scopyhomepage.h"
#include "devicemanager.h"
#include "scannediiocontextcollector.h"
#include "detachedtoolwindowmanager.h"
#include "pluginrepository.h"
#include "scopy-core_export.h"
#include "scopyaboutpage.h"
#include "scopypreferencespage.h"
#include "pluginbase/preferences.h"
#include "pluginbase/versionchecker.h"
#include "iioutil/cyclicaltask.h"
#include "iioutil/iioscantask.h"
#include <gui/widgets/scopystatusbar.h>
#include "versioncheckmessage.h"

#include "toolmenumanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ScopyMainWindow;
}
QT_END_NAMESPACE
namespace scopy {
class ScopyMainWindow_API;
class SCOPY_CORE_EXPORT ScopyMainWindow : public QMainWindow
{
	friend class ScopyMainWindow_API;
	Q_OBJECT

public:
	ScopyMainWindow(QWidget *parent = nullptr);
	~ScopyMainWindow();
	void initAboutPage(PluginManager *pm = nullptr);
	void initPreferencesPage(PluginManager *pm = nullptr);
	void initPreferences();
	void initTranslations();
	void loadPluginsFromRepository(PluginRepository *pr = nullptr);

	void showEvent(QShowEvent *event) override;

public Q_SLOTS:
	void requestTools(QString id);
	void receiveVersionDocument(QJsonDocument document);
	void addDeviceToUi(QString id, Device *d);
	void removeDeviceFromUi(QString);
	void save();
	void load();
	void save(QString file);
	void load(QString file);
	void handlePreferences(QString, QVariant);

private:
	ScopyAboutPage *about;
	ScopyPreferencesPage *prefPage;
	PluginRepository *pr;
	ScopyHomePage *hp;
	DeviceManager *dm;

	CyclicalTask *scanCycle;
	IIOScanTask *scanTask;
	ScannedIIOContextCollector *scc;
	DetachedToolWindowManager *dtm;

	LicenseOverlay *license = nullptr;
	VersionCheckMessage *checkUpdate = nullptr;
	ScopyStatusBar *statusBar;
	ScopyMainWindow_API *api;
	Ui::ScopyMainWindow *ui;
	QOpenGLWidget *m_glLoader;
	ToolMenuManager *m_toolMenuManager;
	ScanButtonController *m_sbc;

	void loadOpenGL();
	void initPythonWIN32();
	void loadDecoders();
	void initApi();
	void initStatusBar();
	void handleScanner();
	void enableScanner();
	void deviceAutoconnect();

protected:
	void closeEvent(QCloseEvent *event) override;
};
} // namespace scopy
#endif // SCOPYMAINWINDOW_H
