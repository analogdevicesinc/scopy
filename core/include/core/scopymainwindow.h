#ifndef SCOPYMAINWINDOW_H
#define SCOPYMAINWINDOW_H

#include <QMainWindow>

#include "license_overlay.h"
#include "scopyhomepage.h"
#include "devicemanager.h"
#include "scannediiocontextcollector.h"
#include "toolmanager.h"
#include "detachedtoolwindowmanager.h"
#include "pluginrepository.h"
#include "scopy-core_export.h"
#include "scopyaboutpage.h"
#include "scopypreferencespage.h"
#include "pluginbase/preferences.h"
#include "versionchecker.h"
#include "iioutil/cyclicaltask.h"
#include "iioutil/iioscantask.h"
#include "versioncheckoverlay.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ScopyMainWindow; }
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
	ScopyAboutPage* about;
	ScopyPreferencesPage* prefPage;
	PluginRepository *pr;
	ScopyHomePage *hp;
	DeviceManager *dm;
	Preferences *pref;

	CyclicalTask *scanCycle;
	IIOScanTask *scanTask;
	ScannedIIOContextCollector *scc;
	ToolManager *toolman;
	DetachedToolWindowManager *dtm;

	LicenseOverlay *license = nullptr;
	VersionCheckOverlay *checkUpdate = nullptr;
	ScopyMainWindow_API *api;
	Ui::ScopyMainWindow *ui;

	void loadOpenGL();
	void initPythonWIN32();
	void loadDecoders();
	void initApi();
protected:
	void closeEvent(QCloseEvent *event) override;
};
}
#endif // SCOPYMAINWINDOW_H
