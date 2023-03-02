#include "scopymainwindow.h"
#include "scanbuttoncontroller.h"
#include "ui_scopymainwindow.h"
#include "scopyhomepage.h"
#include <scopyaboutpage.h>
#include <scopypreferencespage.h>
#include <QLabel>
#include <device.h>
#include <QFileDialog>
#include <QStandardPaths>
#include "pluginbase/preferences.h"
#include "scopycore_config.h"
#ifdef ENABLE_SCOPYJS
#include "scopyjs/scopyjs.h"
#endif
#include "iioutil/contextprovider.h"
#include "pluginbase/messagebroker.h"

using namespace adiscope;
ScopyMainWindow::ScopyMainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::ScopyMainWindow)
{	
	ui->setupUi(this);
	initPreferences();

#ifdef ENABLE_SCOPYJS
	ScopyJS::GetInstance();
#endif
	ContextProvider::GetInstance();
	MessageBroker::GetInstance();

	auto tb = ui->wToolBrowser;
	auto ts = ui->wsToolStack;
	auto tm = tb->getToolMenu();

	hp = new ScopyHomePage(this);
	scanTask = new IIOScanTask(this);
	scanTask->setScanParams("usb");
	scanCycle = new CyclicalTask(scanTask,this);
	scc = new ScannedIIOContextCollector(this);
	pr = new PluginRepository(this);


	pr->init("plugins/plugins");
	PluginManager *pm = pr->getPluginManager();

	initAboutPage(pm);
	initPreferencesPage(pm);

	ScanButtonController *sbc = new ScanButtonController(scanCycle,hp->scanControlBtn(),this);

	dm = new DeviceManager(pm, this);
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
	ts->add("about", about);
	ts->add("preferences", prefPage);

	connect(scanTask,SIGNAL(scanFinished(QStringList)),scc,SLOT(update(QStringList)));

	connect(scc,SIGNAL(foundDevice(QString)),dm,SLOT(addDevice(QString)));
	connect(scc,SIGNAL(lostDevice(QString)),dm,SLOT(removeDevice(QString)));

	connect(hp,SIGNAL(requestDevice(QString)),this,SLOT(requestTools(QString)));

	connect(hp,SIGNAL(requestAddDevice(QString)),dm,SLOT(addDevice(QString)));
	connect(dm,SIGNAL(deviceAdded(QString,Device*)),this,SLOT(addDeviceToUi(QString,Device*)));

	connect(dm,SIGNAL(deviceRemoveStarted(QString, Device*)),this,SLOT(removeDeviceFromUi(QString)));
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

	connect(dm,SIGNAL(requestDevice(QString)),hp,SLOT(viewDevice(QString)));
	connect(dm,SIGNAL(requestTool(QString)),toolman,SLOT(showTool(QString)));

	connect(dm,SIGNAL(deviceChangedToolList(QString,QList<ToolMenuEntry*>)),toolman,SLOT(changeToolListContents(QString,QList<ToolMenuEntry*>)));
	sbc->startScan();

//	dm->addDevice("ip:");
//	dm->addDevice("usb:");

	connect(tb, SIGNAL(requestSave()), this, SLOT(save()));
	connect(tb, SIGNAL(requestLoad()), this, SLOT(load()));

}

void ScopyMainWindow::save() {
	QString selectedFilter;
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save"), "", "", &selectedFilter);
	save(fileName);

}

void ScopyMainWindow::load() {
	QString selectedFilter;
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), "", "", &selectedFilter);
	load(fileName);
}

void ScopyMainWindow::save(QString file)
{
	QSettings s(file, QSettings::Format::IniFormat);
	dm->save(s);
}

void ScopyMainWindow::load(QString file)
{
	QSettings s(file, QSettings::Format::IniFormat);
	dm->load(s);
}

void ScopyMainWindow::requestTools(QString id) {
	toolman->showToolList(id);
}

ScopyMainWindow::~ScopyMainWindow(){

	scanCycle->stop();
	delete ui;
}

void ScopyMainWindow::initAboutPage(PluginManager *pm)
{
	about = new ScopyAboutPage(this);
	if(!pm)
		return;
	QList<Plugin*> plugin = pm->getOriginalPlugins();
	for(Plugin* p : plugin) {
		QString content = p->about();
		if(!content.isEmpty()) {
			about->addHorizontalTab(about->buildPage(content),p->name());
		}
	}
}


void ScopyMainWindow::initPreferencesPage(PluginManager *pm)
{
	prefPage = new ScopyPreferencesPage(this);
	if(!pm)
		return;

	QList<Plugin*> plugin = pm->getOriginalPlugins();
	for(Plugin* p : plugin) {
		p->initPreferences();
		if(p->loadPreferencesPage()) {
			prefPage->addHorizontalTab(p->preferencesPage(),p->name());
		}
	}
}


void ScopyMainWindow::initPreferences()
{
	QString preferencesPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/preferences.ini";
	Preferences *p = Preferences::GetInstance();
	p->setPreferencesFilename(preferencesPath);
	p->load();
	p->init("a","true");
}


void ScopyMainWindow::addDeviceToUi(QString id, Device *d)
{
	hp->addDevice(id,d);
	toolman->addToolList(id,d->toolList());
}

void ScopyMainWindow::removeDeviceFromUi(QString id)
{
	toolman->removeToolList(id);
	hp->removeDevice(id);
}

