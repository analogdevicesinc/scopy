#ifndef SCOPYMAINWINDOW_H
#define SCOPYMAINWINDOW_H

#include <QMainWindow>

#include "scopyhomepage.h"
#include "devicemanager.h"
#include "scannediiocontextcollector.h"
#include "toolmanager.h"
#include "detachedtoolwindowmanager.h"
#include "pluginrepository.h"
#include "scopycore_export.h"
#include "scopyaboutpage.h"
#include "scopypreferencespage.h"
#include "pluginbase/preferences.h"
#include "iioutil/cyclicaltask.h"
#include "iioutil/iioscantask.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ScopyMainWindow; }
QT_END_NAMESPACE
namespace adiscope {
class SCOPYCORE_EXPORT ScopyMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ScopyMainWindow(QWidget *parent = nullptr);
    ~ScopyMainWindow();
    void initAboutPage(PluginManager *pm = nullptr);
    void initPreferencesPage(PluginManager *pm = nullptr);
    void initPreferences();

public Q_SLOTS:
    void requestTools(QString id);
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

    Ui::ScopyMainWindow *ui;

    void loadOpenGL();
protected:
    void closeEvent(QCloseEvent *event) override;
};
}
#endif // SCOPYMAINWINDOW_H
