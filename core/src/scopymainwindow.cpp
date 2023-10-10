#include "scopymainwindow.h"

#include "animationmanager.h"
#include "device.h"
#include "iioutil/contextprovider.h"
#include "logging_categories.h"
#include "pluginbase/messagebroker.h"
#include "pluginbase/preferences.h"
#include "pluginbase/scopyjs.h"
#include "popupwidget.h"
#include "qmessagebox.h"
#include "scanbuttoncontroller.h"
#include "scopy-core_config.h"
#include "scopyaboutpage.h"
#include "scopyhomepage.h"
#include "scopypreferencespage.h"

#include "ui_scopymainwindow.h"

#include <QFileDialog>
#include <QLabel>
#include <QLoggingCategory>
#include <QOpenGLWidget>
#include <QStandardPaths>
#include <QSurfaceFormat>
#include <QTranslator>

#include <common/scopyconfig.h>
#include <functional>
#include <libsigrokdecode/libsigrokdecode.h>
#include <scopymainwindow_api.h>
#include <stylehelper.h>
#include <translationsrepository.h>
#include <utility>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_SCOPY, "Scopy")

ScopyMainWindow::ScopyMainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::ScopyMainWindow)
{
	QElapsedTimer timer;
	timer.start();
	ui->setupUi(this);
	setWindowTitle("Scopy-" + scopy::config::fullversion());

	StyleHelper::GetInstance()->initColorMap();
	setAttribute(Qt::WA_QuitOnClose, true);
	initPythonWIN32();
	initPreferences();

	ContextProvider::GetInstance();
	MessageBroker::GetInstance();

	// get the version document
	auto vc = VersionChecker::GetInstance(); // get VersionCache instance
	vc->subscribe(this,
		      &ScopyMainWindow::receiveVersionDocument); // 'subscribe' to receive the version QJsonDocument

	auto tb = ui->wToolBrowser;
	auto ts = ui->wsToolStack;
	auto tm = tb->getToolMenu();

	scanTask = new IIOScanTask(this);
	scanTask->setScanParams("usb");
	scanCycle = new CyclicalTask(scanTask, this);
	scc = new ScannedIIOContextCollector(this);
	pr = new PluginRepository(this);
	loadPluginsFromRepository(pr);

	PluginManager *pm = pr->getPluginManager();

	initAboutPage(pm);
	initPreferencesPage(pm);
	initTranslations();

	hp = new ScopyHomePage(this, pm);
	ScanButtonController *sbc = new ScanButtonController(scanCycle, hp->scanControlBtn(), this);

	dm = new DeviceManager(pm, this);
	dm->setExclusive(true);

	dtm = new DetachedToolWindowManager(this);
	toolman = new ToolManager(tm, ts, dtm, this);
	toolman->addToolList("home", {});
	toolman->addToolList("add", {});

	connect(tm, &ToolMenu::toggleAttach, toolman, &ToolManager::toggleAttach);
	connect(tb, &ToolBrowser::collapsed, [=](bool coll) {
		ui->animHolder->setAnimMin(50);
		ui->animHolder->toggleMenu(!coll);
	});
	connect(tb, &ToolBrowser::requestTool, ts, &ToolStack::show);
	connect(tb, &ToolBrowser::requestTool, dtm, &DetachedToolWindowManager::show);

	ts->add("home", hp);
	ts->add("about", about);
	ts->add("preferences", prefPage);

	connect(scanTask, SIGNAL(scanFinished(QStringList)), scc, SLOT(update(QStringList)));

	connect(scc, SIGNAL(foundDevice(QString, QString)), dm, SLOT(createDevice(QString, QString)));
	connect(scc, SIGNAL(lostDevice(QString, QString)), dm, SLOT(removeDevice(QString, QString)));

	connect(hp, SIGNAL(requestDevice(QString)), this, SLOT(requestTools(QString)));

	connect(hp, SIGNAL(requestAddDevice(QString, QString)), dm, SLOT(createDevice(QString, QString)));
	connect(dm, SIGNAL(deviceAdded(QString, Device *)), this, SLOT(addDeviceToUi(QString, Device *)));

	connect(dm, SIGNAL(deviceRemoveStarted(QString, Device *)), scc, SLOT(removeDevice(QString, Device *)));
	connect(dm, SIGNAL(deviceRemoveStarted(QString, Device *)), this, SLOT(removeDeviceFromUi(QString)));

	if(dm->getExclusive()) {
		// only for device manager exclusive mode - stop scan on connect
		connect(dm, SIGNAL(deviceConnected(QString, Device *)), sbc, SLOT(stopScan()));
		connect(dm, SIGNAL(deviceDisconnected(QString, Device *)), sbc, SLOT(startScan()));
	}

	connect(dm, SIGNAL(deviceConnected(QString, Device *)), scc, SLOT(lock(QString, Device *)));
	connect(dm, SIGNAL(deviceConnected(QString, Device *)), toolman, SLOT(lockToolList(QString)));
	connect(dm, SIGNAL(deviceConnected(QString, Device *)), hp, SLOT(connectDevice(QString)));
	connect(dm, SIGNAL(deviceDisconnected(QString, Device *)), scc, SLOT(unlock(QString, Device *)));
	connect(dm, SIGNAL(deviceDisconnected(QString, Device *)), toolman, SLOT(unlockToolList(QString)));
	connect(dm, SIGNAL(deviceDisconnected(QString, Device *)), hp, SLOT(disconnectDevice(QString)));

	connect(dm, SIGNAL(requestDevice(QString)), hp, SLOT(viewDevice(QString)));
	connect(dm, SIGNAL(requestTool(QString)), toolman, SLOT(showTool(QString)));

	connect(dm, SIGNAL(deviceChangedToolList(QString, QList<ToolMenuEntry *>)), toolman,
		SLOT(changeToolListContents(QString, QList<ToolMenuEntry *>)));
	sbc->startScan();

	connect(tb, SIGNAL(requestSave()), this, SLOT(save()));
	connect(tb, SIGNAL(requestLoad()), this, SLOT(load()));

	connect(hp, &ScopyHomePage::newDeviceAvailable, dm, &DeviceManager::addDevice);

	initApi();
#ifdef SCOPY_DEV_MODE
	// this is an example of how autoconnect is done

//	 auto id = api->addDevice("m2k","ip:127.0.0.1");
//	 auto id = api->addDevice("iio","ip:10.48.65.163");
//	 auto id = api->addDevice("iio","ip:192.168.2.1");
//	 auto id = api->addDevice("test","");

//	 api->connectDevice(id);
//	 api->switchTool(id, "Oscilloscope");
#endif

	qInfo(CAT_BENCHMARK) << "ScopyMainWindow constructor took: " << timer.elapsed() << "ms";
}

void ScopyMainWindow::save()
{
	QString selectedFilter;
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save"), "", "", &selectedFilter);
	save(fileName);
}

void ScopyMainWindow::load()
{
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

void ScopyMainWindow::closeEvent(QCloseEvent *event) { dm->disconnectAll(); }

void ScopyMainWindow::requestTools(QString id) { toolman->showToolList(id); }

ScopyMainWindow::~ScopyMainWindow()
{

	scanCycle->stop();
	delete ui;
}

void ScopyMainWindow::initAboutPage(PluginManager *pm)
{
	QElapsedTimer timer;
	timer.start();
	about = new ScopyAboutPage(this);
	if(!pm)
		return;
	QList<Plugin *> plugin = pm->getOriginalPlugins();
	for(Plugin *p : plugin) {
		QString content = p->about();
		if(!content.isEmpty()) {
			about->addHorizontalTab(about->buildPage(content), p->name());
		}
	}
	qInfo(CAT_BENCHMARK) << " Init about page took: " << timer.elapsed() << "ms";
}

void ScopyMainWindow::initPreferencesPage(PluginManager *pm)
{
	prefPage = new ScopyPreferencesPage(this);
	if(!pm)
		return;

	QList<Plugin *> plugin = pm->getOriginalPlugins();
	for(Plugin *p : plugin) {
		p->initPreferences();
		if(p->loadPreferencesPage()) {
			prefPage->addHorizontalTab(p->preferencesPage(), p->name());
		}
	}
}

void ScopyMainWindow::initTranslations()
{
	TranslationsRepository *t = TranslationsRepository::GetInstance();
	t->loadTranslations(Preferences::GetInstance()->get("general_language").toString());
}

void ScopyMainWindow::initPreferences()
{
	QElapsedTimer timer;
	timer.start();
	QString preferencesPath = scopy::config::preferencesFolderPath() + "/preferences.ini";
	Preferences *p = Preferences::GetInstance();
	p->setPreferencesFilename(preferencesPath);
	p->load();
	p->init("general_first_run", true);
	p->init("general_save_session", true);
	p->init("general_save_attached", true);
	p->init("general_doubleclick_attach", true);
	p->init("general_use_opengl", true);
	p->init("general_use_animations", true);
	p->init("general_theme", "default");
	p->init("general_language", "en");
	p->init("general_plot_target_fps", "60");
	p->init("general_show_plot_fps", true);
	p->init("general_use_native_dialogs", true);
	p->init("general_additional_plugin_path", "");
	p->init("general_load_decoders", true);
	p->init("general_doubleclick_ctrl_opens_menu", true);
	p->init("general_check_online_version", false);

	connect(p, SIGNAL(preferenceChanged(QString, QVariant)), this, SLOT(handlePreferences(QString, QVariant)));

	if(p->get("general_use_opengl").toBool()) {
		loadOpenGL();
	}
	if(p->get("general_load_decoders").toBool()) {
		loadDecoders();
	}
	if(p->get("general_first_run").toBool()) {
		license = new LicenseOverlay(this);
		checkUpdate = new VersionCheckOverlay(this);

		QMetaObject::invokeMethod(checkUpdate, &VersionCheckOverlay::showOverlay, Qt::QueuedConnection);
		QMetaObject::invokeMethod(license, &LicenseOverlay::showOverlay, Qt::QueuedConnection);
	}
	QString theme = p->get("general_theme").toString();
	QString themeName = "scopy-" + theme;
	QIcon::setThemeName(themeName);
	QIcon::setThemeSearchPaths({":/gui/icons/" + themeName});
	qInfo(CAT_BENCHMARK) << "Init preferences took: " << timer.elapsed() << "ms";
}

void ScopyMainWindow::loadOpenGL()
{

	// this should be part of scopygui
	// set surfaceFormat as in Qt example: HelloGL2 -
	// https://code.qt.io/cgit/qt/qtbase.git/tree/examples/opengl/hellogl2/main.cpp?h=5.15#n81
	QSurfaceFormat fmt;
	//	fmt.setDepthBufferSize(24);
	//	QSurfaceFormat::setDefaultFormat(fmt);

	// This acts as a loader for the OpenGL context, our plots load and draw in the OpenGL context
	// at the same time which causes some race condition and causes the app to hang
	// with this workaround, the app loads the OpenGL context before any plots are created
	// Probably there's a better way to do this
	auto a = new QOpenGLWidget(this);
	qInfo(CAT_SCOPY, "OpenGL loaded");
	a->deleteLater();
}

void ScopyMainWindow::loadPluginsFromRepository(PluginRepository *pr)
{

	QElapsedTimer timer;
	timer.start();
	// Check the local plugins folder first
	QDir pathDir(scopy::config::localPluginFolderPath());
	if(pathDir.exists()) {
		pr->init(scopy::config::localPluginFolderPath());
	} else {
		pr->init(scopy::config::defaultPluginFolderPath());
	}
#ifndef Q_OS_ANDROID
	QString pluginAdditionalPath = Preferences::GetInstance()->get("general_additional_plugin_path").toString();
	if(!pluginAdditionalPath.isEmpty()) {
		pr->init(pluginAdditionalPath);
	}
#endif
	qInfo(CAT_BENCHMARK) << "Loading the plugins from the repository took: " << timer.elapsed() << "ms";
}

void ScopyMainWindow::handlePreferences(QString str, QVariant val)
{
	Preferences *p = Preferences::GetInstance();

	if(str == "general_use_opengl") {
		Q_EMIT p->restartRequired();

	} else if(str == "general_use_animations") {
		AnimationManager::getInstance().toggleAnimations(val.toBool());

	} else if(str == "general_theme") {
		Q_EMIT p->restartRequired();

	} else if(str == "general_language") {
		Q_EMIT p->restartRequired();
	}
}

void ScopyMainWindow::initPythonWIN32()
{
#ifdef WIN32
	QString pythonhome;
	QString pythonpath;

	pythonpath += QCoreApplication::applicationDirPath() + "\\" + PYTHON_VERSION + ";";
	pythonpath += QCoreApplication::applicationDirPath() + "\\" + PYTHON_VERSION + "\\plat-win;";
	pythonpath += QCoreApplication::applicationDirPath() + "\\" + PYTHON_VERSION + "\\lib-dynload;";
	pythonpath += QCoreApplication::applicationDirPath() + "\\" + PYTHON_VERSION + "\\site-packages;";
	QString scopypythonpath = qgetenv("SCOPY_PYTHONPATH");
	pythonpath += scopypythonpath;

#ifdef SCOPY_DEV_MODE
	pythonhome += QString(BUILD_PYTHON_LIBRARY_DIRS) + ";";
	pythonpath += QString(BUILD_PYTHON_LIBRARY_DIRS) + ";";
	pythonpath += QString(BUILD_PYTHON_LIBRARY_DIRS) + "\\plat-win;";
	pythonpath += QString(BUILD_PYTHON_LIBRARY_DIRS) + "\\lib-dynload;";
	pythonpath += QString(BUILD_PYTHON_LIBRARY_DIRS) + "\\site-packages;";
#endif

	qputenv("PYTHONHOME", pythonhome.toLocal8Bit());
	qputenv("PYTHONPATH", pythonpath.toLocal8Bit());

	qInfo(CAT_SCOPY) << "SCOPY_PYTHONPATH: " << scopypythonpath;
	qInfo(CAT_SCOPY) << "PYTHONHOME: " << qgetenv("PYTHONHOME");
	qInfo(CAT_SCOPY) << "PYTHONPATH: " << qgetenv("PYTHONPATH");
#endif
}

void ScopyMainWindow::loadDecoders()
{
	QElapsedTimer timer;
	timer.start();
#if defined(WITH_SIGROK) && defined(WITH_PYTHON)
#if defined __APPLE__
	QString path = QCoreApplication::applicationDirPath() + "/decoders";
#else
	QString path = "decoders";
#endif

	bool success = true;
	static bool srd_loaded = false;
	if(srd_loaded) {
		srd_exit();
	}

	if(srd_init(path.toStdString().c_str()) != SRD_OK) {
		qInfo(CAT_SCOPY) << "ERROR: libsigrokdecode init failed.";
		success = false;
	} else {
		srd_loaded = true;
		/* Load the protocol decoders */
		srd_decoder_load_all();
		auto decoder = srd_decoder_get_by_id("parallel");

		if(decoder == nullptr) {
			success = false;
			qInfo(CAT_SCOPY) << "ERROR: libsigrokdecode load the protocol decoders failed.";
		}
	}

	if(!success) {
		QMessageBox error(this);
		error.setText(
			tr("ERROR: There was a problem initializing libsigrokdecode. Some features may be missing"));
		error.exec();
	}
#else
	qInfo(CAT_SCOPY) << "Python or libsigrokdecode are disabled, can't load decoders";
#endif
	qInfo(CAT_BENCHMARK) << "Loading the decoders took: " << timer.elapsed() << "ms";
}

void ScopyMainWindow::initApi()
{
	api = new ScopyMainWindow_API(this);
	ScopyJS *js = ScopyJS::GetInstance();
	api->setObjectName("scopy");
	js->registerApi(api);
}

void ScopyMainWindow::addDeviceToUi(QString id, Device *d)
{
	toolman->addToolList(id, d->toolList());
	hp->addDevice(id, d);
}

void ScopyMainWindow::removeDeviceFromUi(QString id)
{
	toolman->removeToolList(id);
	hp->removeDevice(id);
}

void ScopyMainWindow::receiveVersionDocument(QJsonDocument document)
{
	qInfo(CAT_SCOPY) << "The upstream scopy version is" << document << "and the current one is" << SCOPY_VERSION;
}

#include "moc_scopymainwindow.cpp"
