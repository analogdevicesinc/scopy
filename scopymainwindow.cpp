#include "scopymainwindow.h"
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

//	 connect(hp,SIGNAL())

	 cs->startScan(2000);
}

ScopyMainWindow::~ScopyMainWindow()
{
	delete ui;
}

void ScopyMainWindow::addDeviceToUi(QString id, Device *d)
{
	hp->addDevice(id,d->name(),d->uri(),d->icon(),d->page());
}

void ScopyMainWindow::removeDeviceFromUi(QString id)
{
	hp->removeDevice(id);

}

