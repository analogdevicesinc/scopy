#ifndef SCOPYMAINWINDOW_H
#define SCOPYMAINWINDOW_H

#include <QMainWindow>

#include "scopyhomepage.h"
#include "devicemanager.h"
#include "iiocontextscanner.h"
#include "scannediiocontextcollector.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ScopyMainWindow; }
QT_END_NAMESPACE
namespace adiscope {
class ScopyMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ScopyMainWindow(QWidget *parent = nullptr);
    ~ScopyMainWindow();
public Q_SLOTS:
    void addToolsToUi(QString id);
    void addDeviceToUi(QString id, Device *d);
    void removeDeviceFromUi(QString);

private:
    ScopyHomePage *hp;
    DeviceManager *dm;
    IIOContextScanner *cs;
    ScannedIIOContextCollector *scc;

    Ui::ScopyMainWindow *ui;
};
}
#endif // SCOPYMAINWINDOW_H
