#include "scopymainwindow.h"
#include "animationmanager.h"
#include "qopenglwidget.h"
#include "qsurfaceformat.h"
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
#include "pluginbase/scopyjs.h"
#include "iioutil/contextprovider.h"
#include "pluginbase/messagebroker.h"
#include "versionchecker.h"
#include "application_restarter.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_SCOPY,"Scopy")

using namespace adiscope;
ScopyMainWindow::ScopyMainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::ScopyMainWindow)
{	
	ui->setupUi(this);

	setAttribute(Qt::WA_QuitOnClose, true);
	initPreferences();

	ScopyJS::GetInstance();
	ContextProvider::GetInstance();
	MessageBroker::GetInstance();
//	auto vc = VersionCache::GetInstance();
//	if(vc->cacheOutdated()) {
//		vc->updateCache();
//		connect(vc,&VersionCache::cacheUpdated,this,[=](){
//			qInfo()<<vc->cache();
//		});
//	}

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

	dtm = new DetachedToolWindowManager(this);
	toolman = new ToolManager(tm,ts,dtm,this);
	toolman->addToolList("home",{});
	toolman->addToolList("add",{});

	connect(tm,&ToolMenu::toggleAttach,toolman,&ToolManager::toggleAttach);
	connect(tb,&ToolBrowser::requestTool,ts, &ToolStack::show);
	connect(tb,&ToolBrowser::requestTool,dtm, &DetachedToolWindowManager::show);

	ts->add("home", hp);
	ts->add("about", about);
	ts->add("preferences", prefPage);

	connect(scanTask,SIGNAL(scanFinished(QStringList)),scc,SLOT(update(QStringList)));

	connect(scc,SIGNAL(foundDevice(QString, QString)),dm,SLOT(createDevice(QString, QString)));
	connect(scc,SIGNAL(lostDevice(QString, QString)),dm,SLOT(removeDevice(QString, QString)));

	connect(hp,SIGNAL(requestDevice(QString)),this,SLOT(requestTools(QString)));

	connect(hp,SIGNAL(requestAddDevice(QString, QString)),dm,SLOT(createDevice(QString, QString)));
	connect(dm,SIGNAL(deviceAdded(QString,Device*)),this,SLOT(addDeviceToUi(QString,Device*)));

	connect(dm,SIGNAL(deviceRemoveStarted(QString, Device*)),scc,SLOT(removeDevice(QString, Device*)));
	connect(dm,SIGNAL(deviceRemoveStarted(QString, Device*)),this,SLOT(removeDeviceFromUi(QString)));
	connect(hp,SIGNAL(requestRemoveDevice(QString)),dm,SLOT(removeDeviceById(QString)));

	if(dm->getExclusive()) {
		// only for device manager exclusive mode - stop scan on connect
		connect(dm,SIGNAL(deviceConnected(QString, Device*)),sbc,SLOT(stopScan()));
		connect(dm,SIGNAL(deviceDisconnected(QString, Device*)),sbc,SLOT(startScan()));
	}

	connect(dm,SIGNAL(deviceConnected(QString, Device*)),scc,SLOT(lock(QString, Device*)));
	connect(dm,SIGNAL(deviceConnected(QString, Device*)),toolman,SLOT(lockToolList(QString)));
	connect(dm,SIGNAL(deviceConnected(QString, Device*)),hp,SLOT(connectDevice(QString)));
	connect(dm,SIGNAL(deviceDisconnected(QString, Device*)),scc,SLOT(unlock(QString, Device*)));
	connect(dm,SIGNAL(deviceDisconnected(QString, Device*)),toolman,SLOT(unlockToolList(QString)));
	connect(dm,SIGNAL(deviceDisconnected(QString, Device*)),hp,SLOT(disconnectDevice(QString)));

	connect(dm,SIGNAL(requestDevice(QString)),hp,SLOT(viewDevice(QString)));
	connect(dm,SIGNAL(requestTool(QString)),toolman,SLOT(showTool(QString)));

	connect(dm,SIGNAL(deviceChangedToolList(QString,QList<ToolMenuEntry*>)),toolman,SLOT(changeToolListContents(QString,QList<ToolMenuEntry*>)));
	sbc->startScan();

//	dm->createDevice("m2k","ip:127.0.0.1");
	dm->createDevice("","ip:test");

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

void ScopyMainWindow::closeEvent(QCloseEvent *event)
{
	dm->disconnectAll();
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
	p->init("general_first_run", true);
	p->init("general_save_session", true);
	p->init("general_save_attached", true);
	p->init("general_doubleclick_attach", true);
	p->init("general_use_opengl", true);
	p->init("general_use_animations", true);
	p->init("general_theme", "dark");
	p->init("general_language", "english");
	p->init("general_plot_target_fps", "60");
	p->init("general_show_plot_fps", true);
	p->init("general_use_native_dialogs", true);

	connect(p,SIGNAL(preferenceChanged(QString,QVariant)),this,SLOT(handlePreferences(QString,QVariant)));

	if(p->get("general_use_opengl").toBool())
		loadOpenGL();
}

void ScopyMainWindow::loadOpenGL() {

	// this should be part of scopygui
	// set surfaceFormat as in Qt example: HelloGL2 - https://code.qt.io/cgit/qt/qtbase.git/tree/examples/opengl/hellogl2/main.cpp?h=5.15#n81
	QSurfaceFormat fmt;
	fmt.setDepthBufferSize(24);
	QSurfaceFormat::setDefaultFormat(fmt);

	// This acts as a loader for the OpenGL context, our plots load and draw in the OpenGL context
	// at the same time which causes some race condition and causes the app to hang
	// with this workaround, the app loads the OpenGL context before any plots are created
	// Probably there's a better way to do this
	auto a = new QOpenGLWidget(this);
	qInfo(CAT_SCOPY, "OpenGL loaded");
	a->deleteLater();
}


void ScopyMainWindow::handlePreferences(QString str,QVariant val) {

	if(str == "general_use_opengl") {
		prefPage->showRestartWidget();
	} else if(str == "general_use_animations") {
		AnimationManager::getInstance().toggleAnimations(val.toBool());

	} else if(str == "general_theme") {
		prefPage->showRestartWidget();

	} else if(str == "general_language") {
		prefPage->showRestartWidget();

	}

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


#include "moc_scopymainwindow.cpp"
