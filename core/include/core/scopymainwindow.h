#ifndef SCOPYMAINWINDOW_H
#define SCOPYMAINWINDOW_H

#include <QMainWindow>

#include "scopyhomepage.h"
#include "devicemanager.h"
#include "iiocontextscanner.h"
#include "scannediiocontextcollector.h"
#include "toolmanager.h"
#include "pluginrepository.h"
#include "scopycore_export.h"

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
public Q_SLOTS:
    void requestTools(QString id);
    void addDeviceToUi(QString id, Device *d);
    void removeDeviceFromUi(QString);

private:
    PluginRepository *pr;
    ScopyHomePage *hp;
    DeviceManager *dm;
    IIOContextScanner *cs;
    ScannedIIOContextCollector *scc;
    ToolManager *toolman;

    Ui::ScopyMainWindow *ui;
};
}
#endif // SCOPYMAINWINDOW_H
