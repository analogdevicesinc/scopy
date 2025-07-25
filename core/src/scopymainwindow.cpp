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

#include <QOpenGLWidget>
#include <QSurfaceFormat>
#include <QLabel>
#include <QFileDialog>
#include <QStandardPaths>
#include <QLoggingCategory>
#include <QTranslator>
#include <QOpenGLFunctions>
#include <browsemenu.h>
#include <deviceautoconnect.h>
#include <style.h>
#include <whatsnewoverlay.h>
#include <pkg-manager/pkgmanager.h>
#include <pkgwidget.h>
#include <pluginstab.h>

#include <common/debugtimer.h>
#include "logging_categories.h"
#include "qmessagebox.h"
#include "scopymainwindow.h"
#include "animationmanager.h"

#include "scanbuttoncontroller.h"
#include "ui_scopymainwindow.h"
#include "scopyhomepage.h"
#include "scopyaboutpage.h"
#include "scopypreferencespage.h"
#include "device.h"

#include <gui/restartdialog.h>
#include "application_restarter.h"
#include "pluginbase/preferences.h"
#include "pluginbase/scopyjs.h"
#include "iioutil/connectionprovider.h"
#include "pluginbase/messagebroker.h"
#include "scopy-core_config.h"
#include "pluginbase/statusbarmanager.h"
#include "scopytitlemanager.h"
#include <common/scopyconfig.h>
#include <translationsrepository.h>
#include <libsigrokdecode/libsigrokdecode.h>
#include <stylehelper.h>
#include <scopymainwindow_api.h>
#include <QVersionNumber>
#include <iioutil/iiounits.h>
#include <gui/widgets/scopysplashscreen.h>

using namespace scopy;
using namespace scopy::gui;

Q_LOGGING_CATEGORY(CAT_SCOPY, "Scopy")

ScopyMainWindow::ScopyMainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::ScopyMainWindow)
	, m_glLoader(nullptr)
{
	DebugTimer benchmark;

	initPreferences();

	QSet<QString> pkgsPath{};
	pkgsPath.insert(scopy::config::defaultPkgFolderPath());
#ifdef SCOPY_DEV_MODE
	pkgsPath.insert(scopy::config::localPkgFolderPath());
#endif
	PkgManager::GetInstance();
	PkgManager::init(pkgsPath);

	initStyle();
	ScopySplashscreen::showMessage("Initializing ui");
	ui->setupUi(this);

	ScopyTitleManager::setMainWindow(this);
	ScopyTitleManager::setApplicationName("Scopy");
	ScopyTitleManager::setScopyVersion("v" + QString(scopy::config::version()));
	ScopyTitleManager::setGitHash(QString(SCOPY_VERSION_GIT));

	IIOUnitsManager::GetInstance();
	setAttribute(Qt::WA_QuitOnClose, true);
	initPythonWIN32();
	initStatusBar();
	setupPreferences();

	ConnectionProvider::GetInstance();
	MessageBroker::GetInstance();

	// get the version document
	auto vc = VersionChecker::GetInstance(); // get VersionCache instance
	vc->subscribe(this,
		      &ScopyMainWindow::receiveVersionDocument); // 'subscribe' to receive the version QJsonDocument

	auto ts = ui->wsToolStack;

	BrowseMenu *browseMenu = new BrowseMenu(ui->wToolBrowser);
	ui->wToolBrowser->layout()->addWidget(browseMenu);
	connect(browseMenu, &BrowseMenu::requestTool, ts, &ToolStack::show, Qt::QueuedConnection);
	connect(browseMenu, SIGNAL(requestLoad()), this, SLOT(load()));
	connect(browseMenu, SIGNAL(requestSave()), this, SLOT(save()));
	connect(browseMenu, &BrowseMenu::collapsed, this, &ScopyMainWindow::collapseToolMenu);

	Style::setBackgroundColor(ui->centralwidget, json::theme::background_primary);

	scanTask = new IIOScanTask(this);
	scanTask->setScanParams("usb");
	scanCycle = new CyclicalTask(scanTask, this);
	scc = new ScannedIIOContextCollector(this);

	loadPluginsFromRepository();

	initAboutPage();
	initPreferencesPage();
	initTranslations();

	ScopySplashscreen::showMessage("Loading homepage");
	hp = new ScopyHomePage(this);
	m_sbc = new ScanButtonController(scanCycle, hp->scanControlBtn(), this);
	connect(hp->scanBtn(), &QPushButton::clicked, this, [=]() { scanTask->run(); });

	DeviceAutoConnect::initPreferences();
	dm = new DeviceManager(this);
	bool general_connect_to_multiple_devices = Preferences::get("general_connect_to_multiple_devices").toBool();
	dm->setExclusive(!general_connect_to_multiple_devices);

	dtm = new DetachedToolWindowManager(this);
	m_toolMenuManager = new ToolMenuManager(ts, dtm, browseMenu->toolMenu(), this);

	PkgWidget *pkgWidget = new PkgWidget(this);

	ts->add("home", hp);
	ts->add("about", about);
	ts->add("preferences", prefPage);
	ts->add("package", pkgWidget);

	connect(scanTask, &IIOScanTask::scanFinished, scc, &ScannedIIOContextCollector::update, Qt::QueuedConnection);

	connect(scc, SIGNAL(foundDevice(QString, QString)), dm, SLOT(createDevice(QString, QString)));
	connect(scc, SIGNAL(lostDevice(QString, QString)), dm, SLOT(removeDevice(QString, QString)));

	connect(hp, SIGNAL(requestDevice(QString)), this, SLOT(requestTools(QString)));

	connect(dm, SIGNAL(deviceAdded(QString, Device *)), this, SLOT(addDeviceToUi(QString, Device *)));

	connect(dm, SIGNAL(deviceRemoveStarted(QString, Device *)), scc, SLOT(removeDevice(QString, Device *)));
	connect(dm, SIGNAL(deviceRemoveStarted(QString, Device *)), this, SLOT(removeDeviceFromUi(QString)));

	connect(dm, SIGNAL(deviceConnecting(QString)), hp, SLOT(connectingDevice(QString)));

	connect(dm, &DeviceManager::deviceConnecting, this, [=]() { handleScanner(); });
	connect(dm, &DeviceManager::deviceConnected, this, [=]() { handleScanner(); });
	connect(dm, &DeviceManager::deviceDisconnecting, this, [=]() { handleScanner(); });
	connect(dm, &DeviceManager::deviceDisconnected, this, [=]() { handleScanner(); });

	connect(dm, SIGNAL(deviceConnected(QString, Device *)), scc, SLOT(lock(QString, Device *)));
	connect(dm, SIGNAL(deviceConnected(QString, Device *)), hp, SLOT(connectDevice(QString)));
	connect(dm, SIGNAL(deviceDisconnected(QString, Device *)), scc, SLOT(unlock(QString, Device *)));
	connect(dm, SIGNAL(deviceDisconnected(QString, Device *)), hp, SLOT(disconnectDevice(QString)));

	connect(dm, SIGNAL(requestDevice(QString)), hp, SLOT(viewDevice(QString)));

	connect(dm, &DeviceManager::deviceChangedToolList, m_toolMenuManager, &ToolMenuManager::changeToolListContents);
	connect(dm, SIGNAL(deviceConnected(QString, Device *)), m_toolMenuManager, SLOT(deviceConnected(QString)));
	connect(dm, SIGNAL(deviceDisconnected(QString, Device *)), m_toolMenuManager,
		SLOT(deviceDisconnected(QString)));
	connect(dm, &DeviceManager::requestTool, m_toolMenuManager, &ToolMenuManager::showMenuItem);
	connect(m_toolMenuManager, &ToolMenuManager::requestToolSelect, ts, &ToolStack::show);
	connect(m_toolMenuManager, &ToolMenuManager::requestToolSelect, dtm, &DetachedToolWindowManager::show);
	connect(m_toolMenuManager, &ToolMenuManager::toolStackChanged, browseMenu, &BrowseMenu::onToolStackChanged);
	connect(hp, &ScopyHomePage::displayNameChanged, m_toolMenuManager, &ToolMenuManager::onDisplayNameChanged);
	connect(ts, &ToolStack::currentChanged, this, [this, ts](int idx) { highlightMenuItem(ts, idx); });
	connect(browseMenu, &BrowseMenu::collapsed, m_toolMenuManager, &ToolMenuManager::menuCollapsed);

	connect(hp, &ScopyHomePage::newDeviceAvailable, dm, &DeviceManager::addDevice);

	connect(prefPage, &ScopyPreferencesPage::refreshDevicesPressed, dm, &DeviceManager::requestConnectedDev);
	connect(dm, &DeviceManager::connectedDevices, prefPage, &ScopyPreferencesPage::updateSessionDevices);

	initApi();
#ifdef SCOPY_DEV_MODE
	// this is an example of how autoconnect is done

	//	 auto id = api->addDevice("ip:127.0.0.1", "m2k");
	//	 auto id = api->addDevice("ip:10.48.65.163", "iio");
	auto id = api->addDevice("ip:192.168.2.1", "iio");
	//	 auto id = api->addDevice("", "test");

	api->connectDevice(id);
	// api->switchTool(id, "Time");
#endif
	if(Preferences::get("autoconnect_previous").toBool())
		deviceAutoconnect();
	prefPage->initSessionDevices();

	if(Preferences::get("general_scan_for_devices").toBool()) {
		scanTask->run();
	}
	enableScanner();

	DEBUGTIMER_LOG(benchmark, "ScopyMainWindow constructor took:");
}

void ScopyMainWindow::initStatusBar()
{
	// clear all margin, except the bottom one, to make room the status bar
	statusBar = new ScopyStatusBar(this);
	ui->mainWidget->layout()->addWidget(statusBar);
}

void ScopyMainWindow::handleScanner()
{
	if(Preferences::get("general_scan_for_devices").toBool()) {
		if(dm->busy()) {
			m_sbc->enableScan(false);
			return;
		}
		if(dm->getExclusive() && dm->connectedDeviceCount() == 1) {
			m_sbc->enableScan(false);
			return;
		}
		m_sbc->enableScan(true);
	} else {
		m_sbc->enableScan(false);
	}
}

void ScopyMainWindow::enableScanner()
{
	bool b = Preferences::get("general_scan_for_devices").toBool();
	hp->setScannerEnable(b);
	handleScanner();
}

void ScopyMainWindow::deviceAutoconnect()
{
	QMap<QString, QVariant> devicesMap = Preferences::get("autoconnect_devices").toMap();
	const QStringList &keys = devicesMap.keys();
	for(const QString &uri : keys) {
		QStringList plugins = devicesMap[uri].toString().split(";");
		auto id = api->addDevice(uri, plugins, "iio");
		if(id.isEmpty()) {
			DeviceAutoConnect::removeDevice(uri);
		} else {
			api->connectDevice(id);
		}
	}
}

void ScopyMainWindow::highlightMenuItem(ToolStack *ts, int idx)
{
	// Highlight the menu item associated with the top widget in the tool stack.
	QWidget *crtWidget = ts->widget(idx);
	QString id = ts->getKey(crtWidget);
	Q_EMIT m_toolMenuManager->toolStackChanged(id);
}

void ScopyMainWindow::collapseToolMenu(bool collapse)
{
	if(collapse) {
		ui->animHolder->setAnimMin(Style::getDimension(json::global::unit_4_5));
	} else {
		ui->animHolder->setAnimMax(230);
	}
	ui->animHolder->toggleMenu(!collapse);
}

void ScopyMainWindow::showWhatsNew()
{
	WhatsNewOverlay *whatsNew = new WhatsNewOverlay(this);
	whatsNew->move(this->rect().center() - whatsNew->rect().center());
	QMetaObject::invokeMethod(whatsNew, &WhatsNewOverlay::showOverlay, Qt::QueuedConnection);
}

void ScopyMainWindow::save()
{
	bool useNativeDialogs = Preferences::get("general_use_native_dialogs").toBool();
	QString selectedFilter;
	QString fileName = QFileDialog::getSaveFileName(
		this, tr("Save"), "", "", &selectedFilter,
		(useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));
	save(fileName);
	ScopyTitleManager::setIniFileName(fileName);
}

void ScopyMainWindow::load()
{
	bool useNativeDialogs = Preferences::get("general_use_native_dialogs").toBool();
	QString selectedFilter;
	QString fileName = QFileDialog::getOpenFileName(
		this, tr("Open"), "", "", &selectedFilter,
		(useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));
	load(fileName);
	ScopyTitleManager::setIniFileName(fileName);
}

void ScopyMainWindow::save(QString file)
{
	QSettings s(file, QSettings::Format::IniFormat);
	saveSession(s);
	dm->save(s);
	ScopyTitleManager::setIniFileName(file);
}

void ScopyMainWindow::load(QString file)
{
	QSettings s(file, QSettings::Format::IniFormat);
	loadSession(s);
	dm->load(s);
	ScopyTitleManager::setIniFileName(file);
}

void ScopyMainWindow::saveSession(QSettings &s)
{
	s.beginGroup("scopy");
	s.setValue("fullversion", config::version());
	s.setValue("version", config::version());
	s.endGroup();
}

void ScopyMainWindow::loadSession(QSettings &s) {}

void ScopyMainWindow::closeEvent(QCloseEvent *event)
{
	DeviceAutoConnect::clear();
	if(Preferences::get("autoconnect_previous").toBool()) {
		dm->saveSessionDevices();
	}
	dm->disconnectAll();
}

void ScopyMainWindow::requestTools(QString id) { m_toolMenuManager->showMenuItem(id); }

ScopyMainWindow::~ScopyMainWindow()
{
	scanCycle->stop();
	delete ui;
}

void ScopyMainWindow::initAboutPage()
{
	DebugTimer benchmark;
	about = new ScopyAboutPage(this);
	if(!PluginRepository::GetInstance()) {
		return;
	}
	QList<Plugin *> plugin = PluginRepository::getOriginalPlugins();
	for(Plugin *p : qAsConst(plugin)) {
		QString content = p->about();
		if(!content.isEmpty()) {
			about->addHorizontalTab(about->buildPage(content), p->name());
		}
	}
	PluginsTab *pluginsTab = new PluginsTab();
	about->addHorizontalTab(pluginsTab, "Plugins info");
	DEBUGTIMER_LOG(benchmark, "Init about page took:");
}

void ScopyMainWindow::initPreferencesPage()
{
	prefPage = new ScopyPreferencesPage(this);
	if(!PluginRepository::GetInstance()) {
		return;
	}

	QList<Plugin *> plugin = PluginRepository::getOriginalPlugins();
	for(Plugin *p : qAsConst(plugin)) {
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

void ScopyMainWindow::setupPreferences()
{
	auto p = Preferences::GetInstance();
	if(p->get("general_use_opengl").toBool()) {
		m_glLoader = new QOpenGLWidget(this);
	}
	if(p->get("general_load_decoders").toBool()) {
		loadDecoders();
	}
	if(p->get("general_show_status_bar").toBool()) {
		StatusBarManager::GetInstance()->setEnabled(true);
	}

	if(p->get("general_first_run").toBool()) {
		license = new LicenseOverlay(this);
		auto versionCheckInfo = new VersionCheckMessage(this);

		StatusBarManager::pushWidget(versionCheckInfo, "Should Scopy check for online versions?");

		QMetaObject::invokeMethod(license, &LicenseOverlay::showOverlay, Qt::QueuedConnection);

		connect(license->getContinueBtn(), &QPushButton::clicked, this, &ScopyMainWindow::showWhatsNew,
			Qt::QueuedConnection);
	}

	if(p->get("general_show_whats_new").toBool() && !p->get("general_first_run").toBool()) {
		showWhatsNew();
	}
}
void ScopyMainWindow::initPreferences()
{
	ScopySplashscreen::showMessage("Initializing preferences");
	DebugTimer benchmark;
	QString preferencesPath = scopy::config::preferencesFolderPath() + "/preferences.ini";
	Preferences *p = Preferences::GetInstance();
	p->setPreferencesFilename(preferencesPath);
	p->load();
	p->init("autoconnect_previous", false);
	p->init("general_first_run", true);
	p->init("general_save_session", true);
	p->init("general_save_attached", true);
	p->init("general_doubleclick_attach", true);
	p->init("general_show_whats_new", true);
#if defined(__linux__) && (defined(__arm__) || defined(__aarch64__))
	p->init("general_use_opengl", false);
#else
	p->init("general_use_opengl", true);
#endif
	p->init("general_use_animations", true);
	p->init("font_scale", "1");
	p->init("general_theme", "Scopy");
	p->init("general_language", "en");
	p->init("show_grid", true);
	p->init("show_graticule", false);
	p->init("iiowidgets_use_lazy_loading", true);
	p->init("general_plot_target_fps", "60");
#ifdef WIN32
	p->init("general_show_plot_fps", true);
#else
	p->init("general_show_plot_fps", false);
#endif
	p->init("general_use_native_dialogs", false);
	p->init("general_additional_plugin_path", "");
	p->init("general_load_decoders", true);
	p->init("general_doubleclick_ctrl_opens_menu", true);
	p->init("general_check_online_version", false);
	p->init("general_show_status_bar", true);
	p->init("general_connect_to_multiple_devices", true);
	p->init("general_scan_for_devices", true);
	p->init("device_menu_item", true);
	p->init("pkg_menu_columns", 1);
	p->init("packages_path", scopy::config::pkgFolderPath());
	p->init("general_use_docking_if_available", true);

	connect(p, SIGNAL(preferenceChanged(QString, QVariant)), this, SLOT(handlePreferences(QString, QVariant)));
	DEBUGTIMER_LOG(benchmark, "Init preferences took:");
}

void ScopyMainWindow::initStyle()
{
	Style *style = Style::GetInstance();
	QString theme = Preferences::get("general_theme").toString();
	style->setPkgsThemes(PkgManager::listFilesInfo(QStringList() << scopy::config::pkgStyleThemesDir(),
						       QStringList() << "*.json"));
	style->setPkgsQss(
		PkgManager::listFilesInfo(QStringList() << scopy::config::pkgStylePropDir(), QStringList() << "*.qss"));
	if(!style->getThemeList().contains(theme)) {
		theme = style->getTheme();
		Preferences::set("general_theme", theme);
	}
	style->init(theme, Preferences::get("font_scale").toFloat());

	QString themeName = "scopy-" + theme;
	QIcon::setThemeSearchPaths({":/gui/icons/" + themeName});
}

void ScopyMainWindow::loadOpenGL()
{
	bool disablePref = false;
	QOpenGLContext *ct = QOpenGLContext::currentContext();
	if(ct) {
		QOpenGLFunctions *glFuncs = ct->functions();
		bool glCtxValid = ct->isValid();
		QString glVersion = QString((const char *)(glFuncs->glGetString(GL_VERSION)));
		qInfo(CAT_BENCHMARK) << "GL_VENDOR " << reinterpret_cast<const char *>(glFuncs->glGetString(GL_VENDOR));
		qInfo(CAT_BENCHMARK) << "GL_RENDERER "
				     << reinterpret_cast<const char *>(glFuncs->glGetString(GL_RENDERER));
		qInfo(CAT_BENCHMARK) << "GL_VERSION " << glVersion;
		qInfo(CAT_BENCHMARK) << "GL_EXTENSIONS "
				     << reinterpret_cast<const char *>(glFuncs->glGetString(GL_EXTENSIONS));
		qInfo(CAT_BENCHMARK) << "QOpenGlContext valid: " << glCtxValid;
		if(!glCtxValid || glVersion.compare("2.0.0", Qt::CaseInsensitive) < 0) {
			disablePref = true;
		}
	} else {
		qInfo(CAT_BENCHMARK) << "QOpenGlContext is invalid";
		disablePref = true;
	}

	qInfo(CAT_BENCHMARK) << "OpenGL load status: " << !disablePref;
	if(disablePref) {
		Preferences::GetInstance()->set("general_use_opengl", false);
		Preferences::GetInstance()->save();
		RestartDialog *restarter = new RestartDialog(this);
		restarter->setDescription(
			"Scopy uses OpenGL for high performance plot rendering. Valid OpenGL context (>v2.0.0) not "
			"detected.\n"
			"Restarting will set Scopy rendering mode to software. This option can be changed from the "
			"Preferences "
			"menu.\n"
			"Please visit the <a "
			"href=https://wiki.analog.com/university/tools/m2k/scopy-troubleshooting> Wiki "
			"Analog page</a> for troubleshooting.");
		connect(restarter, &RestartDialog::restartButtonClicked, [=] {
			ApplicationRestarter::triggerRestart();
			restarter->deleteLater();
		});
		QMetaObject::invokeMethod(restarter, &RestartDialog::showDialog, Qt::QueuedConnection);
	}

	delete m_glLoader;
	m_glLoader = nullptr;
}

void ScopyMainWindow::loadPluginsFromRepository()
{

	DebugTimer benchmark;
	PluginRepository *pr = PluginRepository::GetInstance();
	PluginManager *pm = pr->getPluginManager();
	ScopySplashscreen::setPrefix("Loading plugin: ");

	connect(pm, SIGNAL(startLoadPlugin(QString)), ScopySplashscreen::GetInstance(), SLOT(setMessage(QString)));
	pr->init();
	ScopySplashscreen::setPrefix("");
	disconnect(pm, SIGNAL(startLoadPlugin(QString)), ScopySplashscreen::GetInstance(), SLOT(setMessage(QString)));

#ifndef Q_OS_ANDROID
	QString pluginAdditionalPath = Preferences::GetInstance()->get("general_additional_plugin_path").toString();
	if(!pluginAdditionalPath.isEmpty()) {
		pr->init(pluginAdditionalPath);
	}
#endif

	DEBUGTIMER_LOG(benchmark, "Loading the plugins from the repository took:");
}

void ScopyMainWindow::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	Preferences *p = Preferences::GetInstance();
	if(p->get("general_use_opengl").toBool() && m_glLoader) {
		loadOpenGL();
	}
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
	} else if(str == "font_scale") {
		Q_EMIT p->restartRequired();
	} else if(str == "general_language") {
		Q_EMIT p->restartRequired();
	} else if(str == "general_show_status_bar") {
		StatusBarManager::GetInstance()->setEnabled(val.toBool());
	} else if(str == "plugins_use_debugger_v2") {
		Q_EMIT p->restartRequired();
	} else if(str == "debugger_v2_include_debugfs") {
		Q_EMIT p->restartRequired();
	} else if(str == "general_use_docking_if_available") {
		Q_EMIT p->restartRequired();
	} else if(str == "general_connect_to_multiple_devices") {
		bool general_connect_to_multiple_devices =
			Preferences::get("general_connect_to_multiple_devices").toBool();
		dm->setExclusive(!general_connect_to_multiple_devices);
	} else if(str == "general_scan_for_devices") {
		enableScanner();
	} else if(str == "iio_emu_dir_path") {
		Q_EMIT p->restartRequired();
	} else if(str == "packages_path") {
		Q_EMIT p->restartRequired();
	}
}

void ScopyMainWindow::initPythonWIN32()
{
#ifdef WIN32
	ScopySplashscreen::showMessage("Initializing Python engine");

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
	ScopySplashscreen::showMessage("Loading sigrok decoders");
	DebugTimer benchmark;
#if defined(WITH_SIGROK) && defined(WITH_PYTHON)
#if defined __APPLE__
	QString path = QCoreApplication::applicationDirPath() + "/decoders";
#elif defined(__appimage__)
	QString path = QCoreApplication::applicationDirPath() + "/../lib/decoders";
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
	DEBUGTIMER_LOG(benchmark, "Loading the decoders took:");
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
	bool hasConfigPage = (bool)d->configPage();
	DeviceInfo dInfo = {id, d->displayName(), d->param(), d->iconPixmap(), hasConfigPage, d->toolList()};
	m_toolMenuManager->addMenuItem(dInfo);
	hp->addDevice(id, d);
	auto ts = ui->wsToolStack;
	if(hasConfigPage) {
		ts->add(id, d->configPage());
	}
}

void ScopyMainWindow::removeDeviceFromUi(QString id)
{
	m_toolMenuManager->removeMenuItem(id);
	hp->removeDevice(id);
	auto ts = ui->wsToolStack;
	ts->remove(id);
}

void ScopyMainWindow::receiveVersionDocument(QJsonDocument document)
{
	QJsonValue scopyJson = document["scopy"];
	if(scopyJson.isNull()) {
		qWarning(CAT_SCOPY) << "Could not find the entry \"scopy\" in the json document";
		return;
	}

	QJsonValue scopyVersion = scopyJson["version"];
	if(scopyVersion.isNull()) {
		qWarning(CAT_SCOPY)
			<< R"(Could not find the entry "version" in the "scopy" entry of the json document)";
		return;
	}

	QVersionNumber currentScopyVersion = QVersionNumber::fromString(SCOPY_VERSION).normalized();
	QVersionNumber upstreamScopyVersion =
		QVersionNumber::fromString(scopyVersion.toString().remove(0, 1)).normalized();

	if(upstreamScopyVersion > currentScopyVersion) {
		StatusBarManager::pushMessage(
			"Your Scopy version of outdated. Please consider updating it. The newest version is " +
				upstreamScopyVersion.toString(),
			10000); // 10 sec
	}

	qInfo(CAT_SCOPY) << "The upstream scopy version is" << upstreamScopyVersion << "and the current one is"
			 << currentScopyVersion;
}

#include "moc_scopymainwindow.cpp"
