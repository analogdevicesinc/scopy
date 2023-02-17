#include "scopymainwindow.h"
#include "scanbuttoncontroller.h"
#include "ui_scopymainwindow.h"
#include "scopyhomepage.h"
#include <QLabel>
#include <device.h>

using namespace adiscope;
ScopyMainWindow::ScopyMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ScopyMainWindow)
{	
	ui->setupUi(this);
	 auto tb = ui->wToolBrowser;
	 auto ts = ui->wsToolStack;
	 auto tm = tb->getToolMenu();
	 hp = new ScopyHomePage(this);
	 cs = new IIOContextScanner(this);
	 scc = new ScannedIIOContextCollector(this);
	 dm = new DeviceManager(this);

	 ScanButtonController *sbc = new ScanButtonController(cs,hp->scanControlBtn(),this);

	 dm->setExclusive(true);
	 toolman = new ToolManager(tm,ts,this);
	 toolman->addToolList("home",{});
	 toolman->addToolList("add",{});

	 connect(tm,&ToolMenu::requestAttach,ts,&ToolStack::attachTool);
	 connect(tm,&ToolMenu::requestDetach,ts,&ToolStack::detachTool);
	 connect(ts,&ToolStack::attachSuccesful,tm,&ToolMenu::attachSuccesful);
	 connect(ts,&ToolStack::detachSuccesful,tm,&ToolMenu::detachSuccesful);

	 connect(tb,&ToolBrowser::requestTool,ts, &ToolStack::show);
//	 connect(tb,&ToolBrowser::detach,ts, &ToolStack::showTool);
	 ts->add("home", hp);
	 ts->add("about", new QLabel("about scopy ... "));

	 connect(cs,SIGNAL(scanFinished(QStringList)),scc,SLOT(update(QStringList)));

	 connect(scc,SIGNAL(foundDevice(QString)),dm,SLOT(addDevice(QString)));
	 connect(scc,SIGNAL(lostDevice(QString)),dm,SLOT(removeDevice(QString)));

	 connect(dm,SIGNAL(deviceAdded(QString,Device*)),this,SLOT(addDeviceToUi(QString,Device*)));
	 connect(dm,SIGNAL(deviceRemoved(QString)),this,SLOT(removeDeviceFromUi(QString)));
	 connect(hp,SIGNAL(requestDevice(QString)),this,SLOT(requestTools(QString)));
	 connect(hp,SIGNAL(requestAddDevice(QString)),dm,SLOT(addDevice(QString)));
	 connect(hp,SIGNAL(requestRemoveDevice(QString)),dm,SLOT(removeDevice(QString)));

	 if(dm->getExclusive()) {
		 // only for device manager exclusive mode - stop scan on connect
		 connect(dm,SIGNAL(deviceConnected(QString)),sbc,SLOT(stopScan()));
		 connect(dm,SIGNAL(deviceDisconnected(QString)),sbc,SLOT(startScan()));
	 }

	 connect(dm,SIGNAL(deviceConnected(QString)),scc,SLOT(lock(QString)));
	 connect(dm,SIGNAL(deviceConnected(QString)),toolman,SLOT(lockToolList(QString)));
	 connect(dm,SIGNAL(deviceConnected(QString)),hp,SLOT(connectDevice(QString)));	 
	 connect(dm,SIGNAL(deviceDisconnected(QString)),scc,SLOT(unlock(QString)));
	 connect(dm,SIGNAL(deviceDisconnected(QString)),toolman,SLOT(unlockToolList(QString)));
	 connect(dm,SIGNAL(deviceDisconnected(QString)),hp,SLOT(disconnectDevice(QString)));

	 connect(dm,SIGNAL(deviceChangedToolList(QString,QList<ToolMenuEntry*>)),toolman,SLOT(changeToolListContents(QString,QList<ToolMenuEntry*>)));
	 sbc->startScan();


}

void ScopyMainWindow::requestTools(QString id) {
	toolman->showToolList(id);
}

ScopyMainWindow::~ScopyMainWindow()
{
	cs->stopScan();
	delete ui;
}


void ScopyMainWindow::addDeviceToUi(QString id, Device *d)
{
	hp->addDevice(id,d);
	toolman->addToolList(id,d->toolList());
}

void ScopyMainWindow::removeDeviceFromUi(QString id)
{
	hp->removeDevice(id);
	toolman->removeToolList(id);

}

