/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "logging_categories.h"
#include "config.h"
#include "gui/connectDialog.hpp"
#include "gui/dynamicWidget.hpp"
#include "oscilloscope.hpp"
#include "spectrum_analyzer.hpp"
#include "tool_launcher.hpp"
#include "qtjs.hpp"
#include "jsfileio.h"
#include "gui/dragzone.h"
#include "debugger.h"
#include "manualcalibration.h"
#include "apiobjectmanager.h"
#include "device_widget.hpp"
#include "gui/user_notes.hpp"
#include "external_script_api.hpp"
#include "gui/animationmanager.h"
#include "singletone_wrapper.h"
#include "phonehome.h"
#include "datalogger/datalogger.hpp"

#include "ui_device.h"
#include "ui_tool_launcher.h"

#include <QDebug>
#include <QtConcurrentRun>
#include <QSignalTransition>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>
#include <QStringList>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QDesktopWidget>
#include <QJsonDocument>
#include <QDesktopServices>
#include <QSpacerItem>
#include <QOpenGLWidget>
#if __ANDROID__
#include <QtAndroidExtras/QtAndroid>
#include <QAndroidJniEnvironment>
#endif

#include <iio.h>

#include "tool_launcher_api.hpp"

#include "toolmenu.h"
#include "toolmenuitem.h"

#include <libsigrokdecode/libsigrokdecode.h>

#include <libm2k/m2k.hpp>
#include <libm2k/contextbuilder.hpp>
#include <libm2k/m2kexceptions.hpp>
#include <libm2k/digital/m2kdigital.hpp>
#include "scopyExceptionHandler.h"
#ifdef __ANDROID__
#include <libusb.h>
#endif

#define TIMER_TIMEOUT_MS 5000
#define ALIVE_TIMER_TIMEOUT_MS 5000

using namespace adiscope;
using namespace libm2k::context;
using namespace libm2k::digital;

ToolLauncher* adiscope::tl_ptr;
ToolLauncher* adiscope::getToolLauncherInstance() {
	return tl_ptr;
}

ToolLauncher::ToolLauncher(QString prevCrashDump, QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::ToolLauncher), ctx(nullptr),
	power_control(nullptr), dmm(nullptr), signal_generator(nullptr),
	oscilloscope(nullptr), current(nullptr), filter(nullptr),
	logic_analyzer(nullptr), pattern_generator(nullptr), dio(nullptr),
	network_analyzer(nullptr), spectrum_analyzer(nullptr),data_logger(nullptr), debugger(nullptr),
	manual_calibration(nullptr), tl_api(new ToolLauncher_API(this)),
	dioManager(nullptr),
	notifier(STDIN_FILENO, QSocketNotifier::Read),
	infoWidget(nullptr),
	calib(nullptr),
	skip_calibration(false),
	calibrating(false),
	debugger_enabled(false),
	indexFile(""), deviceInfo(""), pathToFile(""),
	manual_calibration_enabled(false),
	devices_btn_group(new QButtonGroup(this)),
	selectedDev(nullptr),
	m_use_decoders(true),
	menu(nullptr),
	m_useNativeDialogs(true),
	m_m2k(nullptr),
	initialCalibrationFlag(true),
	skip_calibration_if_already_calibrated(true),
	m_adc_tools_failed(false),
	m_dac_tools_failed(false),
	about(nullptr),
	openGlLoaded(false)
      #ifdef __ANDROID__
      ,jnienv(new QAndroidJniEnvironment())
      #endif
{
	if (!isatty(STDIN_FILENO))
		notifier.setEnabled(false);

	ui->setupUi(this);
#ifdef __ANDROID__ // LIBUSB WEAK_AUTHORITY
	libusb_set_option(NULL,LIBUSB_OPTION_ANDROID_JAVAVM,jnienv->javaVM());
	libusb_set_option(NULL,LIBUSB_OPTION_WEAK_AUTHORITY,NULL);
#endif

#ifdef __ANDROID__ // JNI hooks
	registerNativeMethods();
#endif

#ifndef __APPLE__
	setWindowIcon(QIcon(":/icon.ico"));
	QApplication::setWindowIcon(QIcon(":/icon.ico"));
#endif

	// TO DO: remove this when the About menu becomes available
	setWindowTitle(QString("Scopy - ") + QString("v"+QString(PROJECT_VERSION)) + " - " + QString(SCOPY_VERSION_GIT));

	prefPanel = new Preferences(this);
	prefPanel->setVisible(false);

	notesPanel = new UserNotes(this);

	notesPanel->setVisible(false);

	connect(ui->prefBtn, &QPushButton::clicked, [=](){
		swapMenu(static_cast<QWidget*>(prefPanel));
	});
	connect(ui->btnNotes, &QPushButton::clicked, [=](){
		swapMenu(static_cast<QWidget*>(notesPanel));
	});
	about = new ScopyAboutDialog(this);
	connect(ui->btnAbout, &QPushButton::clicked, [=](){
		if(!about)
			about = new ScopyAboutDialog(this);
		about->setModal(false);
		about->show();
		about->raise();
		about->activateWindow();
	});

	connect(prefPanel, &Preferences::reset, this, &ToolLauncher::resetSession);
	connect(prefPanel, &Preferences::notify, this, &ToolLauncher::readPreferences);


	current = ui->homeWidget;

	ui->menu->setMinimumSize(ui->menu->sizeHint());
	devices_btn_group->addButton(ui->btnAdd);
	devices_btn_group->addButton(ui->btnHomepage);

	connect(ui->btnAdd, SIGNAL(toggled(bool)), this, SLOT(btnAdd_toggled(bool)));
	connect(ui->btnHomepage, SIGNAL(toggled(bool)), this, SLOT(btnHomepage_toggled(bool)));

	tl_api->setObjectName(QString::fromStdString(Filter::tool_name(
							     TOOL_LAUNCHER)));

	//option background
	connect(ui->btnHome, SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));
	connect(ui->prefBtn, SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));
	connect(ui->btnNotes, SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));

	ui->saveBtn->parentWidget()->setEnabled(false);
	ui->loadBtn->parentWidget()->setEnabled(true);

	ui->btnHome->toggle();

	loadToolTips(false);

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
	js_engine.installExtensions(QJSEngine::ConsoleExtension);
#endif
	QtJs *js_object = new QtJs(&js_engine);
	js_engine.globalObject().setProperty("fileIO",
					     js_engine.newQObject(new JsFileIo(this)));
	tl_api->js_register(&js_engine);

	connect(&notifier, SIGNAL(activated(int)), this, SLOT(hasText()));

	search_timer = new QTimer();
	connect(search_timer, SIGNAL(timeout()), this, SLOT(search()));
	connect(&watcher, SIGNAL(finished()), this, SLOT(update()));
	search_timer->start(TIMER_TIMEOUT_MS);

	alive_timer = new QTimer();
	connect(alive_timer, SIGNAL(timeout()), this, SLOT(ping()));

	QSettings oldSettings;
	QFile scopy(oldSettings.fileName());
	QFile tempFile(oldSettings.fileName() + ".bak");
	if (tempFile.exists())
		tempFile.remove();

	if(!prevCrashDump.isEmpty())
		scopy.copy(prevCrashDump+".ini");

	scopy.copy(tempFile.fileName());
	settings = new QSettings(tempFile.fileName(), QSettings::IniFormat);
	SingleToneWrapper<QSettings *>::getInstance().setWrapped(settings);

	tl_api->ApiObject::load(*settings);

	ui->menu->setMinimumSize(ui->menu->sizeHint() + QSize(40, 0));

	/* Show a smooth opening when the app starts */
	ui->menu->toggleMenu(true);

	connect(ui->saveBtn, &QPushButton::clicked, this, &ToolLauncher::saveSession);
	connect(ui->loadBtn, &QPushButton::clicked, this, &ToolLauncher::loadSession);
    
	connect(ui->menu, &MenuAnim::finished, [=](bool opened) {
		ui->saveLbl->setVisible(opened);
		ui->loadLbl->setVisible(opened);
		ui->prefBtn->setText(opened ? tr("Preferences") : "");
		menu->hideMenuText(!opened);
	});

	connect(ui->stackedWidget, SIGNAL(moved(int)),
		this, SLOT(pageMoved(int)));

	networkAccessManager = new QNetworkAccessManager(this);
	m_phoneHome = new PhoneHome(settings, prefPanel, networkAccessManager);
	if (prefPanel->getFirst_application_run()) {
		createPhoneHomeMessageBox();
		createLicenseMessageBox();
	}
	connect(prefPanel, &Preferences::requestUpdateCheck, [=]() { m_phoneHome->versionsRequest(true);});
	connect(about, &ScopyAboutDialog::forceCheckForUpdates,[=](){
		m_phoneHome->versionsRequest(true);
	});
	connect(m_phoneHome, SIGNAL(checkUpdatesFinished(qint64)), about, SLOT(updateCheckUpdateLabel(qint64)));


	setupHomepage();
	ui->stackedWidget->setCurrentIndex(0);
	setupAddPage();
	readPreferences();
	this->installEventFilter(this);
	ui->btnConnect->hide();
	// Read preferences and then decide if we load OpenGL
	auto openGLEnvVar = qgetenv("SCOPY_USE_OPENGL");
	if( (bool)openGLEnvVar.toInt()) {
		loadOpenGL();
	}

	_setupToolMenu();

	connect(&calibration_thread_watcher, &QFutureWatcher<QPair<bool, bool>>::finished, [=](){
		QFuture<QPair<bool, bool>> ft = calibration_thread_watcher.future();
		QPair<bool, bool> okc = ft.result();

		menu->getToolMenuItemFor(TOOL_DMM)->setCalibrating(false);
		menu->getToolMenuItemFor(TOOL_OSCILLOSCOPE)->setCalibrating(false);
		menu->getToolMenuItemFor(TOOL_SIGNAL_GENERATOR)->setCalibrating(false);
		menu->getToolMenuItemFor(TOOL_SPECTRUM_ANALYZER)->setCalibrating(false);
		menu->getToolMenuItemFor(TOOL_NETWORK_ANALYZER)->setCalibrating(false);

		if (!okc.second && okc.first) {
			QMetaObject::invokeMethod(selectedDev->infoPage(),
						  "setCalibrationStatusLabel",
						  Qt::QueuedConnection,
						  Q_ARG(QString, tr("Calibrated")));
		}

	});


	const QVector<QString>& uris = searchDevices();
	for (const QString& each : uris)
		addContext(each);

	if (prefPanel->getAutomatical_version_checking_enabled()) {
		m_phoneHome->versionsRequest();
	} else {

	}
#if __ANDROID__
	const QVector<QString> permissions({"android.permission.READ_EXTERNAL_STORAGE",
					    "android.permission.WRITE_EXTERNAL_STORAGE",
					    "android.permission.INTERNET"});

	for(const QString &permission : permissions) {
		auto result = QtAndroid::checkPermission(permission);
		if(result == QtAndroid::PermissionResult::Denied) {
			auto resultHash = QtAndroid::requestPermissionsSync(QStringList({permission}));
			if(resultHash[permission] == QtAndroid::PermissionResult::Denied)
				return;
		}
	}
#endif
	//	    skip_calibration=true;

	//	    QFile f("/sdcard/gnuradio/bla.txt");
	//	    f.open(QIODevice::ReadWrite);
	//	    f.write("blabla");
	//	    f.close();

	// set home icon
	ui->btnHome->setText("Home");
	ui->btnHome->setIcon(QIcon::fromTheme("tool_home"));
	ui->btnHome->setIconSize(QSize(32, 32));

	//	f.open(QIODevice::ReadOnly);
	//	qDebug()<<f.readAll();
	//	f.close();

	tl_ptr = this;
}

void ToolLauncher::createPhoneHomeMessageBox() {
	QMessageBox* msgBox = new QMessageBox(this);

	QSize mSize = msgBox->sizeHint(); // here's what you want, not m.width()/height()
	QRect screenRect = QDesktopWidget().screenGeometry();

	msgBox->setText("Do you want to automatically check for newer Scopy and m2k-firmware versions?<br><br>You can change this anytime from the Preferences menu.");
	//msgBox->setInformativeText("");
	msgBox->setTextInteractionFlags(Qt::LinksAccessibleByMouse);


	QPushButton* yesButton = new QPushButton("Yes");
	msgBox->addButton(yesButton ,QMessageBox::AcceptRole);

	QPushButton* noButton = new QPushButton("No");
	msgBox->addButton(noButton ,QMessageBox::RejectRole);

	msgBox->setModal(false);
	msgBox->show();
	msgBox->activateWindow();
	msgBox->move( QPoint( screenRect.x() + screenRect.width()/2 - mSize.width()/2,
			      screenRect.y() + screenRect.height()/2 - mSize.height()/2 ) );

	connect(yesButton, &QAbstractButton::clicked, [&] () {
		prefPanel->setAutomatical_version_checking_enabled(true);
		prefPanel->setFirst_application_run(false);
	});
	connect(noButton, &QAbstractButton::clicked, [&] () {
		prefPanel->setFirst_application_run(false);
	});
}

void ToolLauncher::createLicenseMessageBox() {
	QMessageBox* msgBox = new QMessageBox(this);

	QSize mSize = msgBox->sizeHint(); // here's what you want, not m.width()/height()
	QRect screenRect = QDesktopWidget().screenGeometry();

	msgBox->setTextFormat(Qt::RichText);
	msgBox->setText("Scopy is distributed under the GNU <a href='https://github.com/analogdevicesinc/scopy/blob/master/LICENSE'>GPLv3</a>, and is copyright Analog Devices. Inc and others. "
			"There are no restrictions or obligations on its use, except that you (the user) agree there is no warranty (per the GPLv3). "
			"If you don't agree to this - do not use this software.<br><br>There are obligations and restrictions on distribution of the "
			"source or binary of this application (and the libraries which it is built with) which are detailed in the "
			"<a href='https://github.com/analogdevicesinc/scopy/blob/master/LICENSE'>GPLv3</a> license. More details of additional licenses "
			"for 3rd party libraries can be found in the \"About\" box. ");

	//msgBox->setInformativeText("");
	msgBox->setTextInteractionFlags(Qt::LinksAccessibleByMouse);


	QPushButton* yesButton = new QPushButton("Yes");
	msgBox->addButton(yesButton ,QMessageBox::AcceptRole);

	QPushButton* noButton = new QPushButton("No");
	msgBox->addButton(noButton ,QMessageBox::RejectRole);

	msgBox->setModal(true);
	msgBox->show();
	msgBox->activateWindow();
	msgBox->move( QPoint( screenRect.x() + screenRect.width()/2 - mSize.width()/2,
			      screenRect.y() + screenRect.height()/2 - mSize.height()/2 ) );

	connect(yesButton, &QAbstractButton::clicked, [&] () {

	});
	connect(noButton, &QAbstractButton::clicked, [&] () {
		QCoreApplication::quit();
	});
}

void ToolLauncher::_setupToolMenu()
{
	menu = new ToolMenu(prefPanel, ui->menuContainer);
	ui->menuContainerLayout->addWidget(menu);

	if (!manual_calibration_enabled) {
		menu->getToolMenuItemFor(TOOL_CALIBRATION)->setToolDisabled(true);
	}

	connect(menu, &ToolMenu::toolSelected,
		this, &ToolLauncher::_toolSelected);

	menu->getButtonGroup()->addButton(ui->btnHome);
	menu->getButtonGroup()->addButton(ui->prefBtn);
	menu->getButtonGroup()->addButton(ui->btnNotes);

	infoWidget = new InfoWidget(this);

#ifndef __ANDROID__
	connect(ui->homeWidget, &DetachDragZone::changeText,
		infoWidget, &InfoWidget::setText);
#endif

	connect(menu, &ToolMenu::enableInfoWidget,
		infoWidget, &InfoWidget::enable);
}

void ToolLauncher::_toolSelected(enum tool tool)
{
	QWidget *selectedTool = nullptr;
	selectedToolId = tool;
	switch(tool) {
	case TOOL_OSCILLOSCOPE:
		selectedTool = oscilloscope;
		break;
	case TOOL_SPECTRUM_ANALYZER:
		selectedTool = spectrum_analyzer;
		break;
	case TOOL_NETWORK_ANALYZER:
		selectedTool = network_analyzer;
		break;
	case TOOL_SIGNAL_GENERATOR:
		selectedTool = signal_generator;
		break;
	case TOOL_LOGIC_ANALYZER:
		selectedTool = logic_analyzer;
		break;
	case TOOL_PATTERN_GENERATOR:
		selectedTool = pattern_generator;
		break;
	case TOOL_DIGITALIO:
		selectedTool = dio;
		break;
	case TOOL_DMM:
		selectedTool = dmm;
		break;
	case TOOL_POWER_CONTROLLER:
		selectedTool = power_control;
		break;
	case TOOL_DEBUGGER:
		selectedTool = debugger;
		break;
	case TOOL_CALIBRATION:
		selectedTool = manual_calibration;
		break;
	case TOOL_DATALOGGER:
		if (data_logger) {
			selectedTool = data_logger->getToolView();
		} else {
			selectedTool = nullptr;
		}
		break;
	case TOOL_LAUNCHER:
		break;
	}

	if (selectedTool) {
		swapMenu(selectedTool);
	}
}


void ToolLauncher::readPreferences()
{
	m_logging_enable = prefPanel->getLogging_enabled();
	m_use_decoders = prefPanel->getDigital_decoders_enabled();
	debugger_enabled = prefPanel->getDebugger_enabled();
	skip_calibration_if_already_calibrated = prefPanel->getSkipCalIfCalibrated();
	ui->btnNotes->setVisible(prefPanel->getUser_notes_active());
	allowExternalScript(prefPanel->getExternal_script_enabled());
	if (manual_calibration) {
		manual_calibration->allowManualCalibScript(manual_calibration_enabled,
							   prefPanel->getManual_calib_script_enabled());
	}

	AnimationManager::getInstance().toggleAnimations(prefPanel->getAnimations_enabled());
}

void ToolLauncher::loadIndexPageFromContent(QString fileLocation)
{
	QFileInfo fileInfo(fileLocation);
	if (fileInfo.exists()) {
		QFile indexFile(fileInfo.filePath());
		indexFile.open(QFile::ReadOnly);
		if (!indexFile.readAll().isEmpty()) {
			index->setSearchPaths(QStringList(fileInfo.dir().absolutePath()));
			indexFile.close();
			index->setSource(QUrl::fromLocalFile(fileInfo.filePath()));
			ui->stackedWidget->removeWidget(ui->stackedWidget->widget(0));
			ui->stackedWidget->insertWidget(0, index);
		} else {
			indexFile.close();
		}
	}
}

void ToolLauncher::pageMoved(int direction)
{
	if (ui->btnAdd->isChecked()) {
		if (!devices.size() && direction > 0) {
			return;
		}
		(direction > 0) ? devices.at(0)->click() : ui->btnHomepage->click();
	} else if (ui->btnHomepage->isChecked()) {
		(direction > 0) ? ui->btnAdd->click() : ui->btnHomepage->click();
	} else {
		int selectedIdx = getDeviceIndex(getSelectedDevice()) + direction;
		if (selectedIdx < (int)devices.size()) {
			if (selectedIdx >= 0) {
				devices.at(selectedIdx)->click();
			} else {
				ui->btnAdd->click();
			}
		}
	}
}

void ToolLauncher::saveSession()
{
	if (ctx) {
		QString fileName = QFileDialog::getSaveFileName(this,
								tr("Save session"), "", tr("Scopy-Files (*.ini)"),
								nullptr, (m_useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));
		QFileInfo fi(fileName);
		qDebug()<<fi.absoluteFilePath();
		if (!fileName.isEmpty()) {
			this->tl_api->save(fileName);
		}
	}
}

void ToolLauncher::allowExternalScript(bool prefEnabled)
{
	if (prefEnabled && debugger_enabled) {
		js_engine.globalObject().setProperty("extern",
						     js_engine.newQObject(new ExternalScript_API(this)));
	} else {
		if (js_engine.globalObject().hasProperty("extern")) {
			js_engine.globalObject().deleteProperty("extern");
		}
	}
}

bool ToolLauncher::getUseDecoders() const
{
	return m_use_decoders;
}

void ToolLauncher::setUseDecoders(bool use_decoders)
{
	m_use_decoders = use_decoders;
	prefPanel->setDigital_decoders_enabled(use_decoders);
}

bool ToolLauncher::hasNativeDialogs() const
{
	return m_useNativeDialogs;
}

void ToolLauncher::setNativeDialogs(bool nativeDialogs)
{
	m_useNativeDialogs = nativeDialogs;
}

void ToolLauncher::loadSession()
{
	QString fileName = QFileDialog::getOpenFileName(this,
							tr("Load session"), "", tr("Scopy-Files (*.ini)"),
							nullptr, (m_useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));
	if (!fileName.isEmpty()) {
		this->tl_api->load(fileName);
		updateHomepage();
		setupHomepage();
	}
}

DeviceWidget* ToolLauncher::getConnectedDevice()
{
	for (auto dev : devices) {
		if (dev->connected()) {
			return dev;
		}
	}
	return nullptr;
}

DeviceWidget* ToolLauncher::getSelectedDevice()
{
	for (size_t i = 0; i < devices.size(); i++) {
		auto dev = devices.at(i);
		if (dev->isChecked()) {
			return dev;
		}
	}
	return nullptr;
}

int ToolLauncher::getDeviceIndex(DeviceWidget *device)
{
	for (size_t i = 0; i < devices.size(); i++) {
		if (devices.at(i) == device) {
			return i;
		}
	}
	return -1;
}

void ToolLauncher::resetSession()
{
	QString uri;
	DeviceWidget *connectedDev = nullptr;
	if (ctx) {
		connectedDev = getConnectedDevice();
		uri = connectedDev->uri();
		this->disconnect();
	}
	pathToFile = "";
	indexFile = "";
	deviceInfo = "";
	if(!ui->btnMenu->isChecked()){
		ui->btnMenu->setChecked(true);
	}
	updateHomepage();
	setupHomepage();

	QSettings settings;
	QFile fileScopy(settings.fileName());
	QFile fileBak(settings.fileName() + ".bak");
	fileScopy.open(QFile::WriteOnly);
	fileBak.open(QFile::WriteOnly);
	fileScopy.resize(0);
	fileBak.resize(0);

	if (connectedDev) {
		connectedDev->setChecked(true);
		connectedDev->connectButton()->click();
	}
}

void ToolLauncher::saveSettings()
{
	QSettings settings;
	QFile tempFile(settings.fileName() + ".bak");
	QSettings tempSettings(tempFile.fileName(), QSettings::IniFormat);
	QFile scopyFile(settings.fileName());
	if (scopyFile.exists())
		scopyFile.remove();
	tempSettings.sync();
	QFile::copy(tempFile.fileName(), scopyFile.fileName());
}

void ToolLauncher::runProgram(const QString& program, const QString& fn)
{
	QJSValue val = js_engine.evaluate(program, fn);

	int ret = EXIT_SUCCESS;
	if (val.isError()) {
		qInfo() << "Exception:" << val.toString();
		ret = EXIT_FAILURE;
	} else if (!val.isUndefined()) {
		qInfo() << val.toString();
	}

	/* Exit application */
	qApp->exit(ret);
}

void ToolLauncher::search()
{
	search_timer->stop();
	future = QtConcurrent::run(this, &ToolLauncher::searchDevices);
	watcher.setFuture(future);
}

QVector<QString> ToolLauncher::searchDevices()
{
	struct iio_context_info **info;
	unsigned int nb_contexts;
	QVector<QString> uris;

	struct iio_scan_context *scan_ctx = iio_create_scan_context("usb", 0);

	if (!scan_ctx) {
		std::cerr << "Unable to create scan context!" << std::endl;
		return uris;
	}

	ssize_t ret = iio_scan_context_get_info_list(scan_ctx, &info);

	if (ret < 0) {
		std::cerr << "Unable to scan!" << std::endl;
		goto out_destroy_context;
	}

	nb_contexts = static_cast<unsigned int>(ret);

	for (unsigned int i = 0; i < nb_contexts; i++)
		uris.append(QString(iio_context_info_get_uri(info[i])));

	iio_context_info_list_free(info);
out_destroy_context:
	iio_scan_context_destroy(scan_ctx);
	return uris;
}

void ToolLauncher::updateListOfDevices(const QVector<QString>& uris)
{
	//Delete devices that are in the devices list but not found anymore when scanning

	int pos = 0;
	while (pos < devices.size()) {
		auto dev = devices.at(pos);
		if (dev->connected()) {
			pos++;
			continue;
		}
		QString uri = dev->uri();
		if (uri.startsWith("usb:") && !uris.contains(uri)) {
			ui->stackedWidget->removeWidget(dev->infoPage());
			devices_btn_group->removeButton(dev->deviceButton());
			if (dev->isChecked()) {
				if (pos > 0) {
					devices.at(pos-1)->click();
				} else {
					ui->btnHomepage->click();
				}
			}
			delete dev;
			devices.erase(devices.begin() + pos);
		} else {
			pos++;
		}
	}

	for (const QString& uri : uris) {
		if (!uri.startsWith("usb:"))
			continue;

		auto dev = getDevice(uri);

		if (!dev)
			addContext(uri);
	}

	search_timer->start(TIMER_TIMEOUT_MS);
}

void ToolLauncher::loadToolTips(bool connected){
	if (connected){
		ui->btnHome->setToolTip(QString(tr("Click to open the home menu")));
		ui->btnConnect->setToolTip(QString(tr("Click to disconnect the device")));
		ui->saveBtn->setToolTip(QString(tr("Click to save the current session")));
		ui->loadBtn->setToolTip(QString(tr("Click to load a session")));
	} else {
		ui->btnHome->setToolTip(QString());
		ui->saveBtn->setToolTip(QString());
		ui->loadBtn->setToolTip(QString(tr("Click to load a session")));
		ui->btnConnect->setToolTip(QString(tr("Select a device first")));
	}
}

void ToolLauncher::update()
{
	updateListOfDevices(watcher.result());
}

ToolLauncher::~ToolLauncher()
{

	disconnect();
	this->removeEventFilter(this);

	for (auto it = devices.begin(); it != devices.end(); ++it) {
		delete *it;
	}

	devices.clear();

	delete search_timer;
	delete alive_timer;

	delete infoWidget;
	delete m_phoneHome;
	tl_api->ApiObject::save(*settings);
	m_sessionInfo.save(*settings);

	delete settings;
	SingleToneWrapper<QSettings *>::getInstance().setWrapped(nullptr);

	delete tl_api;
	delete ui;

	saveSettings();
	tl_ptr = nullptr;
}

void ToolLauncher::forgetDeviceBtn_clicked(QString uri)
{
	if (previousIp == uri.mid(3)) {
		previousIp = "";
	}
	DeviceWidget *dev = nullptr;
	for(auto d : devices) {
		if (d == sender()) {
			dev = d;
		}
	}
	if (!dev) {
		return;
	}
	int pos = getDeviceIndex(dev);
	if (dev->uri().startsWith("usb:")) {
		return;
	}

	auto connectedDev = getConnectedDevice();
	if (connectedDev == dev) {
		if (ctx) {
			connectedDev->setConnected(false, false);
			disconnect();
			connectedDev->connectButton()->setToolTip(QString(tr("Click to connect the device")));
		}
	}

	if (dev->uri() == uri) {
		/* Remove device selection and select the
		 * precedent device (or the add page)
		 */
		ui->stackedWidget->removeWidget(dev->infoPage());
		devices_btn_group->removeButton(dev->deviceButton());
		if (dev->isChecked()) {
			if (pos > 0) {
				devices.at(pos-1)->click();
			} else {
				ui->btnHomepage->click();
			}
		}
		delete dev;
		devices.erase(devices.begin() + pos);
	}
}


QPushButton *ToolLauncher::addContext(const QString& uri)
{
	auto tempCtx = iio_create_context_from_uri(uri.toStdString().c_str());
	if (!tempCtx)
		return nullptr;

	auto tempFilter = new Filter(tempCtx);
	if (!tempFilter)
		return nullptr;

	DeviceWidget *deviceWidget = nullptr;
	if (tempFilter->hw_name().compare("M2K") == 0) {
		deviceWidget = DeviceBuilder::newDevice(DeviceBuilder::M2K,
							uri, tempFilter->hw_name(), this);
	} else {
		deviceWidget = DeviceBuilder::newDevice(DeviceBuilder::GENERIC,
							uri, tempFilter->hw_name(), this);
	}

	delete tempFilter;
	iio_context_destroy(tempCtx);
	tempCtx = nullptr;

	auto connectBtn = deviceWidget->connectButton();

	if (connectBtn) {
		connect(connectBtn, SIGNAL(clicked(bool)),
			this, SLOT(connectBtn_clicked(bool)), Qt::QueuedConnection);
	}

	connect(deviceWidget, SIGNAL(forgetDevice(QString)),
		this, SLOT(forgetDeviceBtn_clicked(QString)));

	connect(deviceWidget, SIGNAL(selected(bool)),
		this, SLOT(deviceBtn_clicked(bool)));

	connect(deviceWidget->infoPage(), SIGNAL(stopSearching(bool)),
		this, SLOT(stopSearching(bool)));

	ui->devicesList->insertWidget(ui->devicesList->count() - 1,
				      deviceWidget);
	ui->stackedWidget->addWidget(deviceWidget->infoPage());
	devices_btn_group->addButton(deviceWidget->deviceButton());
	devices.push_back(deviceWidget);

	return deviceWidget->deviceButton();
}

void ToolLauncher::stopSearching(bool stop)
{
	if (stop){
		search_timer->stop();
	} else {
		if (!getConnectedDevice())
			search_timer->start(TIMER_TIMEOUT_MS);
	}
}

void ToolLauncher::btnHomepage_toggled(bool toggled)
{
	setDynamicProperty(ui->homepageWidget, "selected", toggled);
	if (toggled) {
		ui->stackedWidget->slideToIndex(0);
		if (menu && !getConnectedDevice()) {
			menu->loadToolsFromFilter(nullptr);
		}
	}
}

void ToolLauncher::btnAdd_toggled(bool toggled)
{
	setDynamicProperty(ui->addWidget, "selected", toggled);
	if (toggled) {
		ui->stackedWidget->slideToIndex(1);
		if (menu && !getConnectedDevice()) {
			menu->loadToolsFromFilter(nullptr);
		}
	} else {
		if (connectWidget) {
			connectWidget->discardSettings();
		}
	}
}

DeviceWidget* ToolLauncher::getDevice(QString uri)
{
	for (auto dev : devices) {
		if (dev->uri() == uri) {
			return dev;
		}
	}
	return nullptr;
}

void ToolLauncher::highlightDevice(QPushButton *btn)
{
	bool initialBtnState = btn->property("checked").toBool();

	QTimer::singleShot(200, [=](){
		setDynamicProperty(btn, "checked", !initialBtnState);
	});
	QTimer::singleShot(400, [=](){
		setDynamicProperty(btn, "checked", initialBtnState);
	});
	QTimer::singleShot(600, [=](){
		setDynamicProperty(btn, "checked", !initialBtnState);
	});
	QTimer::singleShot(800, [=](){
		setDynamicProperty(btn, "checked", initialBtnState);
	});
}

void ToolLauncher::setupHomepage()
{
	// Welcome page

	QWidget *homepage = new QWidget(ui->stackedWidget);
	QVBoxLayout *layout = new QVBoxLayout(homepage);
	QLabel* versionLabel = new QLabel(this);

	versionLabel->setText(tr("Auto update checks not enabled. Check preferences menu."));
	connect(m_phoneHome, &PhoneHome::scopyVersionChanged, this, [=] () {
		if (m_phoneHome->getScopyVersion().isEmpty()) {
			//versionLabel->setText(tr("Unable to check update server!"));
		} else if (m_phoneHome->getScopyVersion() != QString("v" + QString(PROJECT_VERSION))) {
			versionLabel->setText(tr("Version ") + m_phoneHome->getScopyVersion() + " of Scopy was released. " +
					      "<a href=\"" + m_phoneHome->getScopyLink() +
					      tr("\">Click to update </a>"));
			versionLabel->setTextFormat(Qt::RichText);
			versionLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
			versionLabel->setOpenExternalLinks(true);
		} else {
			versionLabel->setText(tr("Scopy is up to date!"));
		}
		versionLabel->setVisible(true);
	});

	connect(m_phoneHome, &PhoneHome::scopyVersionCheckRequested, this ,[=]() {
		versionLabel->setText(tr("Checking server for updates ... "));
	});

	welcome = new QTextBrowser(homepage);
	welcome->setFrameShape(QFrame::NoFrame);
	welcome->setOpenExternalLinks(true);
	welcome->setSource(QUrl("qrc:/scopy.html"));
	layout->addWidget(welcome);

	QWidget *reportRegion = new QWidget(homepage);
	QHBoxLayout *reportLayout = new QHBoxLayout(reportRegion);
	QPushButton *reportBtn = new QPushButton(reportRegion);
	reportBtn->setText(tr("Report a bug!"));
	reportBtn->setStyleSheet("QPushButton {"
			     "border: 0px;"
			     "border-radius: 4px;"
			     "background-color: #4a64ff;"
			     "color: #ffffff;"
			     "font-size: 14px;}"
		     "QPushButton:hover"
		     "{background-color: #4a34ff;}");
	reportBtn->setMinimumHeight(30);
	reportBtn->setMinimumWidth(100);
	reportLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed));
	reportLayout->addWidget(reportBtn);
	layout->addWidget(reportRegion);

	connect(reportBtn, &QPushButton::clicked, [=](){
		const QUrl url("https://wiki.analog.com/university/tools/m2k/scopy/report");
		QDesktopServices::openUrl(url);
	});

	if (ui->stackedWidget->count() == (devices.size()+2)) {
		ui->stackedWidget->removeWidget(0);
	}
	ui->stackedWidget->insertWidget(0, homepage);
	ui->btnHomepage->setChecked(true);

	// Index page
	index = new QTextBrowser(ui->stackedWidget);
	index->setFrameShape(QFrame::NoFrame);

	layout->addWidget(versionLabel);
	versionLabel->raise();
	versionLabel->setVisible(true);

	if (indexFile == "") {
		return;
	}

	loadIndexPageFromContent(indexFile);
}

void ToolLauncher::setupAddPage()
{
	connectWidget = new ConnectDialog(ui->stackedWidget);
	connect(connectWidget, &ConnectDialog::newContext,
		[=](const QString& uri) {
		auto dev = getDevice(uri);
		if (dev) {
			highlightDevice(dev->deviceButton());
		} else {
			auto btn = addContext(uri);
			if (btn) {
				btn->setChecked(true);
			}
		}
	});

	ui->stackedWidget->insertWidget(1, connectWidget);
}

void ToolLauncher::updateHomepage()
{
	ui->stackedWidget->removeWidget(ui->stackedWidget->widget(0));
}

void ToolLauncher::swapMenu(QWidget *menu)
{
	if (current) {
		current->setVisible(false);
		ui->centralLayout->removeWidget(current);
	}

	current = menu;

	ui->centralLayout->addWidget(current);
	current->setVisible(true);
}

void ToolLauncher::setButtonBackground(bool on)
{
	auto *btn = static_cast<QPushButton *>(QObject::sender());

	setDynamicProperty(btn->parentWidget(), "selected", on);
}

void ToolLauncher::btnOscilloscope_clicked()
{
	swapMenu(static_cast<QWidget *>(oscilloscope));
}

void ToolLauncher::btnSignalGenerator_clicked()
{
	swapMenu(static_cast<QWidget *>(signal_generator));
}

void ToolLauncher::btnDMM_clicked()
{
	swapMenu(static_cast<QWidget *>(dmm));
}

void ToolLauncher::btnPowerControl_clicked()
{
	swapMenu(static_cast<QWidget *>(power_control));
}

void ToolLauncher::btnLogicAnalyzer_clicked()
{
	swapMenu(static_cast<QWidget *>(logic_analyzer));
}

void adiscope::ToolLauncher::btnPatternGenerator_clicked()
{
	swapMenu(static_cast<QWidget *>(pattern_generator));
}

void adiscope::ToolLauncher::btnNetworkAnalyzer_clicked()
{
	swapMenu(static_cast<QWidget *>(network_analyzer));
}

void adiscope::ToolLauncher::btnSpectrumAnalyzer_clicked()
{
	swapMenu(static_cast<QWidget *>(spectrum_analyzer));
}

void adiscope::ToolLauncher::btnDigitalIO_clicked()
{
	swapMenu(static_cast<QWidget *>(dio));
}

void adiscope::ToolLauncher::on_btnHome_clicked()
{
	swapMenu(ui->homeWidget);
}

void adiscope::ToolLauncher::btnDebugger_clicked()
{
	swapMenu(static_cast<QWidget *>(debugger));
}

void adiscope::ToolLauncher::btnCalibration_clicked()
{
	swapMenu(static_cast<QWidget *>(manual_calibration));
}

void adiscope::ToolLauncher::resetStylesheets()
{
	setDynamicProperty(ui->btnConnect, "connected", false);
	setDynamicProperty(ui->btnConnect, "failed", false);

	for (auto dev : devices) {
		dev->setConnected(false, false);
	}
}

void adiscope::ToolLauncher::deviceBtn_clicked(bool pressed)
{
	DeviceWidget *dev = nullptr;
	for (auto d : devices) {
		if (d == sender()) {
			dev = d;
			break;
		}
	}

	deviceInfo = "";
	if (dev && pressed) {
		ui->stackedWidget->slideToIndex(getDeviceIndex(dev) + 2);
	}

	if (pressed && !getConnectedDevice()) {
		if (dev) {
			auto tempCtx = iio_create_context_from_uri(dev->uri().toStdString().c_str());
			if (tempCtx) {
				auto tempFilter = new Filter(tempCtx);
				menu->loadToolsFromFilter(tempFilter);
				if (tempFilter->hw_name().compare("M2K") != 0) {
					menu->getToolMenuItemFor(TOOL_DEBUGGER)->setToolDisabled(false);
				} else {
					if (!debugger_enabled) {
						menu->getToolMenuItemFor(TOOL_DEBUGGER)->setToolDisabled(true);
					}
				}
				delete tempFilter;
				iio_context_destroy(tempCtx);
			}
		}

	}

	if (pressed){
		ui->btnConnect->setToolTip(QString(tr("Click to connect the device")));
	} else {
		ui->btnConnect->setToolTip(QString(tr("Select a device first")));
	}
}

void adiscope::ToolLauncher::disconnect()
{
	/* Switch back to home screen */
	ui->btnHome->click();

	QObject::disconnect(this, SIGNAL(calibrationFailed()),
			    this, SLOT(calibrationFailedCallback()));
	QObject::disconnect(this, SIGNAL(calibrationDone()),
			    this, SLOT(restartToolsAfterCalibration()));

	if (ctx) {
		if (calibrating) {
			calib->cancelCalibration();
			calibration_thread.waitForFinished();
		}
		auto iio = iio_manager::has_instance(filter->device_name(TOOL_DMM));
		if (iio) {
			iio->stop_all();
		}
		alive_timer->stop();

		ui->saveBtn->parentWidget()->setEnabled(false);

		destroyContext();
		loadToolTips(false);
		resetStylesheets();
		auto infoPg = selectedDev->infoPage();
		if (infoPg) {
			infoPg->setConnectionStatusLabel("Not connected");
			infoPg->setCalibrationStatusLabel("");
		}
		search_timer->start(TIMER_TIMEOUT_MS);
		selectedDev->infoPage()->setConnectionStatusLabel("Not connected");
		selectedDev->infoPage()->setCalibrationStatusLabel("");
	}

	/* Update the list of devices now */
	updateListOfDevices(searchDevices());
}

void adiscope::ToolLauncher::ping()
{
	auto dev = iio_context_get_device(ctx, 0);
	const iio_device* test_device = nullptr;

	int ret = iio_device_get_trigger(dev, &test_device);

	if (ret < 0 && ret != -ENOENT) {
		disconnect();
	}
}

void adiscope::ToolLauncher::connectBtn_clicked(bool pressed)
{
	auto connectedDev = getConnectedDevice();
	for (auto d : devices) {
		if (d->connectButton() == sender()) {
			selectedDev = d;
		}
		else {
			d->connectButton()->setEnabled(false);
		}
		d->setConnected(false, false);
	}
	selectedDev->connectButton()->setEnabled(false);
	if (selectedDev != connectedDev) {
		selectedDev->connectButton()->setText(tr("Connecting..."));
	} else {
		selectedDev->connectButton()->setText(tr("Disconnecting..."));
	}
	QApplication::processEvents();

	/* Disconnect connected device, if any */
	if (ctx) {
		QObject::disconnect(connectedDev->calibrateButton(),
				    SIGNAL(clicked()),this, SLOT(requestCalibration()));
		QObject::disconnect(&calibration_thread_watcher, SIGNAL(finished()),
				    this, SLOT(calibrationThreadWatcherFinished()));
		connectedDev->setConnected(false, false);
		disconnect();
		connectedDev->connectButton()->setToolTip(QString(tr("Click to connect the device")));
		connectedDev->connectButton()->setText(tr("Connect"));
	}

	if (connectedDev != selectedDev) {
		/* Connect to the selected device, if any */
		if (selectedDev) {
#ifdef LIBM2K_ENABLE_LOG
			QString path = QFileInfo(settings->fileName()).absolutePath();
			path.append("/ScopyLog-");
			if (m_logging_enable) {
				// logging enabled
				google::SetLogDestination(google::GLOG_INFO, "");
				google::SetLogDestination(google::GLOG_INFO, path.toStdString().c_str());
				// do not create a symlink
				google::SetLogSymlink(google::GLOG_INFO, "");
				// log build info
				QFile fileBuildInfo(":/buildinfo.html");
				fileBuildInfo.open(QIODevice::ReadOnly | QIODevice::Text);
				auto buildInfo = QString(fileBuildInfo.readAll());
				fileBuildInfo.close();
				buildInfo = buildInfo.remove(QRegExp("<(/?)(body|pre)>"));
				LOG(INFO) << buildInfo.toLocal8Bit().toStdString();
			} else {
				// logging disabled
				google::SetLogDestination(google::GLOG_INFO, "");
			}
#endif
			QString uri = selectedDev->uri();
			selectedDev->infoPage()->identifyDevice(false);
			search_timer->stop();
			bool success = switchContext(uri);
			if (success) {
				selectedDev->setConnected(true, false, ctx);
				selectedDev->setName(filter->hw_name());
				selectedDev->infoPage()->identifyDevice(true);
				setDynamicProperty(ui->btnConnect, "connected", true);

				alive_timer->start(ALIVE_TIMER_TIMEOUT_MS);
				ui->saveBtn->parentWidget()->setEnabled(true);
			} else {
				setDynamicProperty(ui->btnConnect, "failed", true);
				selectedDev->setConnected(false, true);
				selectedDev->connectButton()->setEnabled(true);
				disconnect();
			}

			Q_EMIT connectionDone(success);
		}
	} else {
		selectedDev->connectButton()->setEnabled(true);
	}
	for (auto d : devices) {
		if (d->connectButton() != sender()) {
			d->connectButton()->setEnabled(true);
		}
	}
}

void adiscope::ToolLauncher::destroyContext()
{
	if (dio) {
		delete dio;
		dio = nullptr;
	}

	if (dmm) {
		delete dmm;
		dmm = nullptr;
	}

	if (data_logger) {
		delete data_logger;
		data_logger = nullptr;
	}

	if (power_control) {
		delete power_control;
		power_control = nullptr;
	}

	if (signal_generator) {
		delete signal_generator;
		signal_generator = nullptr;
	}

	if (oscilloscope) {
		delete oscilloscope;
		oscilloscope = nullptr;
	}

	if (logic_analyzer) {
		delete logic_analyzer;
		logic_analyzer = nullptr;
	}

	if (pattern_generator) {
		delete pattern_generator;
		pattern_generator = nullptr;
	}

	if (network_analyzer) {
		delete network_analyzer;
		network_analyzer = nullptr;
	}

	if (spectrum_analyzer) {
		delete spectrum_analyzer;
		spectrum_analyzer = nullptr;
	}

	if (debugger) {
		delete debugger;
		debugger = nullptr;
	}

	if (manual_calibration) {
		delete manual_calibration;
		manual_calibration = nullptr;
	}

	if (filter) {
		delete filter;
		filter = nullptr;
	}

	if (calib) {
		delete calib;
		calib = nullptr;
	}

	if (ctx) {
		auto dev = getConnectedDevice();
		if (dev)
			dev->setConnected(false, false);
		if (m_m2k) {
			try {
				libm2k::context::contextClose(m_m2k);
			} catch (libm2k::m2k_exception &e) {
				HANDLE_EXCEPTION(e);
				qDebug() << e.what();
			}
			m_m2k = nullptr;
		} else {
			libm2k::context::contextCloseAll();
		}
		iio_context_destroy(ctx);

		ctx = nullptr;
	}

	if (dioManager) {
		delete dioManager;
		dioManager = nullptr;
	}

	toolList.clear();
}

bool ToolLauncher::loadDecoders(QString path)
{
	static bool srd_loaded = false;

	if (srd_loaded) {
		srd_exit();
	}

	if (srd_init(path.toStdString().c_str()) != SRD_OK) {
		qDebug(CAT_TOOL_LAUNCHER) << "ERROR: libsigrokdecode init failed.";
		return false;
	} else {
		srd_loaded = true;
		/* Load the protocol decoders */
		srd_decoder_load_all();
		auto decoder = srd_decoder_get_by_id("parallel");

		if (decoder == nullptr) {
			return false;
		}
	}

	return true;
}

void adiscope::ToolLauncher::saveRunningInputTools() {
	if (dmm->isRunning())
		running_tools.push_back(dmm);
	if (oscilloscope->isRunning())
		running_tools.push_back(oscilloscope);
	if (logic_analyzer->isRunning())
		running_tools.push_back(logic_analyzer);
	if (spectrum_analyzer->isRunning())
		running_tools.push_back(spectrum_analyzer);
}

void adiscope::ToolLauncher::saveRunningTools() {
	for(auto &tool : toolList) {
		if(tool->isRunning()) {
			running_tools.push_back(tool);
		}
	}
}

int adiscope::ToolLauncher::getRunningToolsCount() {
	int i = 0;
	for(auto &tool : toolList) {
		if(tool->isRunning()) {
			i++;
		}
	}
	return i;
}

bool adiscope::ToolLauncher::getCtx()
{
	return ctx;
}

void adiscope::ToolLauncher::stopRunningTools() {
	for(auto &tool : running_tools) {
		tool->stop();
	}
}

void adiscope::ToolLauncher::restoreRunningTools() {
	while(!running_tools.empty())
	{
		Tool* tool = running_tools.back();
		running_tools.pop_back();
		tool->run();
	}
}


void adiscope::ToolLauncher::saveRunningToolsBeforeCalibration()
{
	if (dmm->isRunning()) calibration_saved_tools.push_back(dmm);
	if (oscilloscope->isRunning()) calibration_saved_tools.push_back(oscilloscope);
	if (signal_generator->isRunning()) calibration_saved_tools.push_back(signal_generator);
	if (spectrum_analyzer->isRunning()) calibration_saved_tools.push_back(spectrum_analyzer);
	if (network_analyzer->isRunning()) calibration_saved_tools.push_back(network_analyzer);
	menu->getToolMenuItemFor(TOOL_DMM)->setCalibrating(true);
	menu->getToolMenuItemFor(TOOL_OSCILLOSCOPE)->setCalibrating(true);
	menu->getToolMenuItemFor(TOOL_SIGNAL_GENERATOR)->setCalibrating(true);
	menu->getToolMenuItemFor(TOOL_SPECTRUM_ANALYZER)->setCalibrating(true);
	menu->getToolMenuItemFor(TOOL_NETWORK_ANALYZER)->setCalibrating(true);
	menu->getToolMenuItemFor(TOOL_DMM)->setDisabled(true);
	menu->getToolMenuItemFor(TOOL_OSCILLOSCOPE)->setDisabled(true);
	menu->getToolMenuItemFor(TOOL_SIGNAL_GENERATOR)->setDisabled(true);
	menu->getToolMenuItemFor(TOOL_SPECTRUM_ANALYZER)->setDisabled(true);
	menu->getToolMenuItemFor(TOOL_NETWORK_ANALYZER)->setDisabled(true);
}

void adiscope::ToolLauncher::stopToolsBeforeCalibration()
{
	for(Tool* tool : qAsConst(calibration_saved_tools))
		tool->stop();
}

void adiscope::ToolLauncher::restartToolsAfterCalibration()
{
	menu->getToolMenuItemFor(TOOL_DMM)->setCalibrating(false);
	menu->getToolMenuItemFor(TOOL_OSCILLOSCOPE)->setCalibrating(false);
	menu->getToolMenuItemFor(TOOL_SIGNAL_GENERATOR)->setCalibrating(false);
	menu->getToolMenuItemFor(TOOL_SPECTRUM_ANALYZER)->setCalibrating(false);
	menu->getToolMenuItemFor(TOOL_NETWORK_ANALYZER)->setCalibrating(false);
	menu->getToolMenuItemFor(TOOL_DMM)->setEnabled(true);
	menu->getToolMenuItemFor(TOOL_OSCILLOSCOPE)->setEnabled(true);
	menu->getToolMenuItemFor(TOOL_SIGNAL_GENERATOR)->setEnabled(true);
	menu->getToolMenuItemFor(TOOL_SPECTRUM_ANALYZER)->setEnabled(true);
	menu->getToolMenuItemFor(TOOL_NETWORK_ANALYZER)->setEnabled(true);
	auto dev = getConnectedDevice();
	if (!dev) {
		return;
	}
	dev->calibrateButton()->setEnabled(true);
	while(!calibration_saved_tools.empty())
	{
		Tool* tool = calibration_saved_tools.back();
		calibration_saved_tools.pop_back();
		tool->run();
	}
}
void adiscope::ToolLauncher::requestCalibration()
{
	getConnectedDevice()->calibrateButton()->setEnabled(false);
	saveRunningToolsBeforeCalibration();
	stopToolsBeforeCalibration();

	calibration_thread = QtConcurrent::run(std::bind(&ToolLauncher::calibrate,
							 this));
}

void adiscope::ToolLauncher::requestCalibrationCancel()
{
	calib->cancelCalibration();
	getConnectedDevice()->calibrateButton()->setEnabled(true);
}

void adiscope::ToolLauncher::calibrationSuccessCallback()
{
}

void adiscope::ToolLauncher::calibrationFailedCallback()
{
	selectedDev->infoPage()->setCalibrationStatusLabel(tr("Calibration Failed"));
	selectedDev->connectButton()->setText(tr("Disconnect"));
	selectedDev->connectButton()->setEnabled(true);
	getConnectedDevice()->calibrateButton()->setEnabled(true);
}

QPair<bool, bool> adiscope::ToolLauncher::initialCalibration()
{
	QPair<bool, bool> okc = {true, false};

	if (!skip_calibration) {
		initialCalibrationFlag = true;
		okc = calibrate();
		initialCalibrationFlag = false;
	}

	//	if (okc.first) {
	//		Q_EMIT adcCalibrationDone();
	//		Q_EMIT dacCalibrationDone();
	//		Q_EMIT calibrationDone();
	//	}
	//	else {
	//		Q_EMIT calibrationFailed();
	//	}

	return okc;
}

QPair<bool, bool> adiscope::ToolLauncher::calibrate()
{
	bool ok=false;
	calibrating = true;

	bool skipCalib = false;

	QString statusLabel;
	if (calib->isInitialized()) {
		if (prefPanel->getAttemptTempLutCalib() && calib->hasContextCalibration()) {
			float calibTemperature = calib->calibrateFromContext();

			statusLabel = tr("Calibrated from look-up table @ ") + QString::number(calibTemperature) + " deg. Celsius";
			skipCalib = true;
			ok = true;

		} else {
			// always calibrate if initial flag is set
			// if it's calibrated and skip_calibration_if_calibrated - do not calibrate
			if (!(initialCalibrationFlag && skip_calibration_if_already_calibrated && calib->isCalibrated() )) {
				statusLabel = tr("Calibrating ... ");
				ok = calib->calibrateAll();
			} else {
				statusLabel = tr("Calibration skipped because already calibrated.");
				skipCalib = true;
				ok = true;
			}
		}
	}
	QMetaObject::invokeMethod(selectedDev->infoPage(),
				  "setCalibrationStatusLabel",
				  Qt::QueuedConnection,
				  Q_ARG(QString, statusLabel));

	calibrating = false;

	if (ok) {
		Q_EMIT adcCalibrationDone();
		Q_EMIT dacCalibrationDone();
		Q_EMIT calibrationDone();
	}
	else {
		Q_EMIT calibrationFailed();
	}

	return { ok, skipCalib };
}

void adiscope::ToolLauncher::enableAdcBasedTools()
{
	try {
		if (filter->compatible(TOOL_OSCILLOSCOPE)) {
			oscilloscope = new Oscilloscope(ctx, filter, menu->getToolMenuItemFor(TOOL_OSCILLOSCOPE),
							&js_engine, this);
			toolList.push_back(oscilloscope);
			adc_users_group.addButton(menu->getToolMenuItemFor(TOOL_OSCILLOSCOPE)->getToolStopBtn());
			connect(oscilloscope, &Oscilloscope::showTool, [=]() {
				menu->getToolMenuItemFor(TOOL_OSCILLOSCOPE)->getToolBtn()->click();
			});
			if (logic_analyzer) {
				oscilloscope->setLogicAnalyzer(logic_analyzer);
			}
		}

		if (filter->compatible(TOOL_DMM)) {
			dmm = new DMM(ctx, filter, menu->getToolMenuItemFor(TOOL_DMM),
				      &js_engine, this);
			adc_users_group.addButton(menu->getToolMenuItemFor(TOOL_DMM)->getToolStopBtn());
			toolList.push_back(dmm);
			connect(dmm, &DMM::showTool, [=]() {
				menu->getToolMenuItemFor(TOOL_DMM)->getToolBtn()->click();
			});
		}

		if (filter->compatible(TOOL_CALIBRATION)) {
			manual_calibration = new ManualCalibration(ctx, filter,menu->getToolMenuItemFor(TOOL_CALIBRATION),
								   &js_engine, this, calib);
			adc_users_group.addButton(menu->getToolMenuItemFor(TOOL_CALIBRATION)->getToolStopBtn());
			toolList.push_back(manual_calibration);
		}

		if (filter->compatible(TOOL_SPECTRUM_ANALYZER)) {
			spectrum_analyzer = new SpectrumAnalyzer(ctx, filter, menu->getToolMenuItemFor(TOOL_SPECTRUM_ANALYZER),&js_engine, this);
			toolList.push_back(spectrum_analyzer);
			adc_users_group.addButton(menu->getToolMenuItemFor(TOOL_SPECTRUM_ANALYZER)->getToolStopBtn());
			connect(spectrum_analyzer, &SpectrumAnalyzer::showTool, [=]() {
				menu->getToolMenuItemFor(TOOL_SPECTRUM_ANALYZER)->getToolBtn()->click();
			});
		}

		if (filter->compatible((TOOL_NETWORK_ANALYZER))) {

			network_analyzer = new NetworkAnalyzer(ctx, filter, menu->getToolMenuItemFor(TOOL_NETWORK_ANALYZER), &js_engine, this);
			adc_users_group.addButton(menu->getToolMenuItemFor(TOOL_NETWORK_ANALYZER)->getToolStopBtn());
			toolList.push_back(network_analyzer);
			connect(network_analyzer, &NetworkAnalyzer::showTool, [=]() {
				menu->getToolMenuItemFor(TOOL_NETWORK_ANALYZER)->getToolBtn()->click();
			});
			network_analyzer->setOscilloscope(oscilloscope);
		}

		m_adc_tools_failed = false;
		Q_EMIT adcToolsCreated();
	} catch (libm2k::m2k_exception &e) {
		qDebug(CAT_TOOL_LAUNCHER) << e.what();
		m_adc_tools_failed = true;
	}
}


void adiscope::ToolLauncher::enableDacBasedTools()
{
	try {
		if (filter->compatible(TOOL_SIGNAL_GENERATOR)) {
			signal_generator = new SignalGenerator(ctx, filter,
							       menu->getToolMenuItemFor(TOOL_SIGNAL_GENERATOR), &js_engine, this);
			toolList.push_back(signal_generator);
			connect(signal_generator, &SignalGenerator::showTool, [=]() {
				menu->getToolMenuItemFor(TOOL_SIGNAL_GENERATOR)->getToolBtn()->click();
			});
		}
		if (pathToFile != "") {
			this->tl_api->load(pathToFile);
		}

		m_dac_tools_failed = false;
		Q_EMIT dacToolsCreated();
		selectedDev->connectButton()->setText(tr("Disconnect"));
		selectedDev->connectButton()->setEnabled(true);

		for (auto &tool : toolList) {
			tool->setNativeDialogs(m_useNativeDialogs);
			qDebug() << tool << " will use native dialogs: " << m_useNativeDialogs;
		}
	} catch (libm2k::m2k_exception &e) {
		qDebug(CAT_TOOL_LAUNCHER) << e.what();
		m_dac_tools_failed = true;
	}

	if (m_adc_tools_failed || m_dac_tools_failed) {
		disconnect();
	}
}

bool adiscope::ToolLauncher::switchContext(const QString& uri)
{
	destroyContext();

	if (uri.startsWith("ip:")) {
		previousIp = uri.mid(3);
	}

	auto dev = getDevice(uri);
	if (dev->infoPage()->ctx()) {
		ctx = dev->infoPage()->ctx();
	} else {
		ctx = iio_create_context_from_uri(uri.toStdString().c_str());
	}

	if (!ctx) {
		return false;
	}

	m_m2k = m2kOpen(ctx, "");
#ifdef LIBM2K_ENABLE_LOG
	if (m_m2k) {
	    m_m2k->logAllAttributes();
	}
#endif
	filter = new Filter(ctx);
	menu->loadToolsFromFilter(filter);
	if (filter->hw_name().compare("M2K") != 0) {
		menu->getToolMenuItemFor(TOOL_DEBUGGER)->setToolDisabled(false);
	} else {
		if (!debugger_enabled) {
			menu->getToolMenuItemFor(TOOL_DEBUGGER)->setToolDisabled(true);
		}
	}

	calib = new Calibration(ctx, &js_engine);
	calib->initialize();

	try {
		if (filter->compatible(TOOL_PATTERN_GENERATOR)
				|| filter->compatible(TOOL_DIGITALIO)) {
			dioManager = new DIOManager(ctx, filter);
		}
		if (filter->compatible(TOOL_LOGIC_ANALYZER)
				|| filter->compatible(TOOL_PATTERN_GENERATOR)) {

			if (!m_use_decoders) {
				search_timer->stop();

				QMessageBox info(this);
				info.setText(tr("Digital decoders support is disabled. Some features may be missing"));
				info.exec();
			} else {
#if defined __APPLE__
				bool success = loadDecoders(QCoreApplication::applicationDirPath() + "/decoders");
#else
				bool success = loadDecoders("decoders");
#endif

				if (!success) {
					search_timer->stop();

					QMessageBox error(this);
					error.setText(tr("There was a problem initializing libsigrokdecode. Some features may be missing"));
					error.exec();
				}

			}
		}

		if (filter->compatible(TOOL_DIGITALIO)) {
			dio = new DigitalIO(nullptr, filter, menu->getToolMenuItemFor(TOOL_DIGITALIO),
					    dioManager, &js_engine, this);
			toolList.push_back(dio);
			connect(dio, &DigitalIO::showTool, [=]() {
				menu->getToolMenuItemFor(TOOL_DIGITALIO)->getToolBtn()->click();
			});
		}

		if (filter->compatible(TOOL_DEBUGGER)) {
			debugger = new Debugger(ctx, filter,menu->getToolMenuItemFor(TOOL_DEBUGGER),
						&js_engine, this);
			QObject::connect(debugger, &Debugger::newDebuggerInstance, this,
					 &ToolLauncher::addDebugWindow);
		}

		if (filter->compatible(TOOL_DATALOGGER)) {
			 data_logger = new DataLogger(ctx, filter, menu->getToolMenuItemFor(TOOL_DATALOGGER),&js_engine, this);
			 toolList.push_back(data_logger);
		}


		if (filter->compatible(TOOL_POWER_CONTROLLER)) {
			power_control = new PowerController(ctx, menu->getToolMenuItemFor(TOOL_POWER_CONTROLLER),
							    &js_engine, this);
			toolList.push_back(power_control);
			connect(power_control, &PowerController::showTool, [=]() {
				menu->getToolMenuItemFor(TOOL_POWER_CONTROLLER)->getToolBtn()->click();
			});
		}

		if (filter->compatible(TOOL_LOGIC_ANALYZER)) {
			logic_analyzer = new logic::LogicAnalyzer(ctx, filter, menu->getToolMenuItemFor(TOOL_LOGIC_ANALYZER),
								  &js_engine, this);
			toolList.push_back(logic_analyzer);
			connect(logic_analyzer, &logic::LogicAnalyzer::showTool, [=]() {
				menu->getToolMenuItemFor(TOOL_LOGIC_ANALYZER)->getToolBtn()->click();
			});
		}

		if (filter->compatible((TOOL_PATTERN_GENERATOR))) {
			pattern_generator = new logic::PatternGenerator(ctx, filter,
									menu->getToolMenuItemFor(TOOL_PATTERN_GENERATOR), &js_engine, dioManager, this);
			toolList.push_back(pattern_generator);
			connect(pattern_generator, &logic::PatternGenerator::showTool, [=]() {
				menu->getToolMenuItemFor(TOOL_PATTERN_GENERATOR)->getToolBtn()->click();
			});
		}
	}

	catch (libm2k::m2k_exception &e) {
		return false;
	}

	connect(menu->getToolMenuItemFor(TOOL_NETWORK_ANALYZER)->getToolStopBtn(),
		&QPushButton::toggled,
		[=](bool en) {
		if(en) {
			if(!menu->getToolMenuItemFor(TOOL_SIGNAL_GENERATOR)->getToolStopBtn()->isChecked())
				return;
			menu->getToolMenuItemFor(TOOL_SIGNAL_GENERATOR)->getToolStopBtn()->setChecked(false);
		}
	});
	connect(menu->getToolMenuItemFor(TOOL_SIGNAL_GENERATOR)->getToolStopBtn(),
		&QPushButton::toggled,
		[=](bool en) {
		if(en) {
			if(adc_users_group.checkedId() == adc_users_group.id(menu->getToolMenuItemFor(TOOL_NETWORK_ANALYZER)->getToolStopBtn())){
				auto btn = dynamic_cast<CustomPushButton*>(
							menu->getToolMenuItemFor(TOOL_NETWORK_ANALYZER)->getToolStopBtn());
				btn->setChecked(false);
			}
		}
	});

	loadToolTips(true);

	connect(this, SIGNAL(adcCalibrationDone()),
		this, SLOT(enableAdcBasedTools()));
	connect(this, SIGNAL(dacCalibrationDone()),
		this, SLOT(enableDacBasedTools()));
	connect(this, SIGNAL(calibrationFailed()),
		this, SLOT(calibrationFailedCallback()));
	connect(this, SIGNAL(calibrationFailed()),
		this, SLOT(calibrationSuccessCallback()));
	connect(this, SIGNAL(calibrationDone()),
		this, SLOT(restartToolsAfterCalibration()));

	selectedDev->calibrateButton()->setEnabled(false);

	menu->getToolMenuItemFor(TOOL_DMM)->setCalibrating(true);
	menu->getToolMenuItemFor(TOOL_OSCILLOSCOPE)->setCalibrating(true);
	menu->getToolMenuItemFor(TOOL_SIGNAL_GENERATOR)->setCalibrating(true);
	menu->getToolMenuItemFor(TOOL_SPECTRUM_ANALYZER)->setCalibrating(true);
	menu->getToolMenuItemFor(TOOL_NETWORK_ANALYZER)->setCalibrating(true);

	selectedDev->infoPage()->setCalibrationStatusLabel(tr("Calibrating"));

	calibration_thread = QtConcurrent::run(std::bind(&ToolLauncher::initialCalibration,
							 this));

	calibration_thread_watcher.setFuture(calibration_thread);
	connect(&calibration_thread_watcher, SIGNAL(finished()), this, SLOT(calibrationThreadWatcherFinished()));

	return true;
}

void ToolLauncher::calibrationThreadWatcherFinished()
{
	QObject::disconnect(this, SIGNAL(adcCalibrationDone()),
			    this, SLOT(enableAdcBasedTools()));
	QObject::disconnect(this, SIGNAL(dacCalibrationDone()),
			    this, SLOT(enableDacBasedTools()));

	auto dev = getConnectedDevice();
	if (dev) {
		dev->calibrateButton()->setEnabled(true);
		connect(dev->calibrateButton(), SIGNAL(clicked()),this, SLOT(requestCalibration()));
	}
}

void ToolLauncher::hasText()
{
	QTextStream in(stdin);
	QTextStream out(stdout);

	js_cmd.append(in.readLine());

	unsigned int nb_open_braces = js_cmd.count(QChar('{'));
	unsigned int nb_closing_braces = js_cmd.count(QChar('}'));

	if (nb_open_braces == nb_closing_braces) {
		QJSValue val = js_engine.evaluate(js_cmd);

		if (val.isError()) {
			out << "Exception:" << val.toString() << Qt::endl;
		} else if (!val.isUndefined()) {
			out << val.toString() << Qt::endl;
		}

		js_cmd.clear();
		out << "scopy > ";
	} else {
		js_cmd.append(QChar('\n'));

		out << "> ";
	}

	out.flush();
}

void ToolLauncher::checkIp(const QString& ip)
{
	if (iio_create_network_context(ip.toStdString().c_str())) {
		previousIp = ip;

		QString uri = "ip:" + ip;

		bool found = false;
		for (auto dev : devices) {
			if (dev->uri() == uri) {
				found = true;
				break;
			}
		}

		if (!found)
			QMetaObject::invokeMethod(this, "addContext",
						  Qt::QueuedConnection,
						  Q_ARG(const QString&, uri));
	} else {
		previousIp = "";
	}
}

void ToolLauncher::toolDetached(bool detached)
{
	Tool *tool = static_cast<Tool *>(QObject::sender());

	if (detached) {
		/* Switch back to the home screen */
		if (current == static_cast<QWidget *>(tool->getCentralWidget()))
			ui->btnHome->click();

		setDynamicProperty(tool->runButton()->parentWidget(), "selected", false);
	}

	tool->getCentralWidget()->setVisible(detached);

	tool->getCentralWidget()->setMinimumSize(910, 490);
}

enum tool ToolLauncher::getSelectedToolId() const {
	return selectedToolId;
}



void ToolLauncher::closeEvent(QCloseEvent *event)
{
	// Notify tools that the launcher is closing
	Q_EMIT launcherClosed();

	// Close all detached windows
	QApplication::closeAllWindows();

	for (auto iterator : qAsConst(debugInstances)) {
		delete iterator;
	}
	debugInstances.clear();

	for (auto iterator : qAsConst(debugWindows)) {
		iterator->close();
		delete iterator;
	}
	debugWindows.clear();
}

Preferences *ToolLauncher::getPrefPanel() const
{
	return prefPanel;
}

Calibration *ToolLauncher::getCalibration() const
{
	return calib;
}

PhoneHome *ToolLauncher::getPhoneHome() const
{
	return m_phoneHome;
}

bool ToolLauncher::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonRelease) {
		QMouseEvent *me = static_cast<QMouseEvent *>(event);
		QPoint mousePosLoad = ui->Load->mapFromGlobal(QCursor::pos());
		QPoint mousePosSave = ui->Save->mapFromGlobal(QCursor::pos());
		QPoint mousePosPref = ui->preference->mapFromGlobal(QCursor::pos());
		QPoint mousePosNotes = ui->notes->mapFromGlobal(QCursor::pos());

		if (ui->menuControl->rect().contains(me->pos())) {
			ui->btnMenu->toggle();
			return true;
		} else if (ui->Load->rect().contains(mousePosLoad)) {
			ui->loadBtn->click();
			return true;
		} else  if (ui->Save->rect().contains(mousePosSave)) {
			ui->saveBtn->click();
			return true;
		} else if (ui->preference->rect().contains(mousePosPref)) {
			ui->prefBtn->click();
			return true;
		} else if (ui->notes->rect().contains(mousePosNotes)) {
			ui->btnNotes->click();
			return true;
		}
	}

#ifdef __ANDROID__
	else if(event->type() == QEvent::KeyRelease) {
			QKeyEvent *ke = static_cast<QKeyEvent *>(event);
			if (ke->key() == Qt::Key_Back) {

				QMessageBox* msgBox = new QMessageBox(this);
				QSize mSize = msgBox->sizeHint(); // here's what you want, not m.width()/height()
				QRect screenRect = QDesktopWidget().screenGeometry();

				msgBox->setText("Are you sure you want to close Scopy?");
				msgBox->setTextInteractionFlags(Qt::NoTextInteraction);

				QPushButton* yesButton = new QPushButton("Yes");
				msgBox->addButton(yesButton ,QMessageBox::AcceptRole);

				QPushButton* noButton = new QPushButton("No");
				msgBox->addButton(noButton ,QMessageBox::RejectRole);

				msgBox->setModal(false);
				msgBox->show();
				msgBox->activateWindow();
				msgBox->move( QPoint( screenRect.x() + screenRect.width()/2 - mSize.width()/2,
						      screenRect.y() + screenRect.height()/2 - mSize.height()/2 ) );

				connect(yesButton, &QAbstractButton::clicked, [&] () {
					// TODO: call save preferences
					qApp->exit();
				});

				connect(noButton, &QAbstractButton::clicked, [&] () {
					event->ignore();
				});
			}
			return true;
		}
#endif

	return QObject::eventFilter(watched, event);
}

#ifdef __ANDROID__

void ToolLauncher::saveSessionJavaHelper(JNIEnv *env, jobject /*thiz*/) {
	qDebug()<<"-- Saving session JNI";
	ToolLauncher* tl = getToolLauncherInstance();
	if(tl)
	{
		getToolLauncherInstance()->tl_api->sync();
		getToolLauncherInstance()->saveSettings();
	}
}


void ToolLauncher::saveAndStopRunningInputToolsJNI(JNIEnv *env, jobject /*thiz*/) {
	qDebug()<<"-- Saving and stopping input tools JNI";
	ToolLauncher* tl = getToolLauncherInstance();
	if(tl)
	{
		getToolLauncherInstance()->saveRunningInputTools();
		getToolLauncherInstance()->stopRunningTools();
	}
}

void ToolLauncher::saveAndStopRunningToolsJNI(JNIEnv *env, jobject /*thiz*/) {
	qDebug()<<"-- Saving and stopping tools JNI";
	ToolLauncher* tl = getToolLauncherInstance();
	if(tl)
	{
		getToolLauncherInstance()->saveRunningTools();
		getToolLauncherInstance()->stopRunningTools();
	}
}

void ToolLauncher::restoreRunningToolsJNI(JNIEnv *env, jobject /*thiz*/) {
	qDebug()<<"-- Saving and stopping tools JNI";
	ToolLauncher* tl = getToolLauncherInstance();
	if(tl)
	{
		getToolLauncherInstance()->restoreRunningTools();
	}
}

int ToolLauncher::nrOfToolsSavedJNI(JNIEnv *env, jobject /*thiz*/) {
	qDebug()<<"-- Getting number of stopped tools JNI";
	ToolLauncher* tl = getToolLauncherInstance();
	if(tl)
	{
		int val = getToolLauncherInstance()->running_tools.size();
		qDebug()<<"saved: "<<val;
		return val;
	}
	return 0;
}

int ToolLauncher::nrOfToolsRunningJNI(JNIEnv *env, jobject /*thiz*/) {
	qDebug()<<"-- Getting number of stopped tools JNI";
	ToolLauncher* tl = getToolLauncherInstance();
	if(tl)
	{
		int val = getToolLauncherInstance()->getRunningToolsCount();
		qDebug()<<"saved: "<<val;
		return val;
	}
	return 0;
}

bool ToolLauncher::hasCtxJNI(JNIEnv *env, jobject /*thiz*/)
{
	qDebug()<<"-- Getting number of stopped tools JNI";
	ToolLauncher* tl = getToolLauncherInstance();
	if(tl)
	{
		bool val = getToolLauncherInstance()->getCtx();
		qDebug()<<"has Ctx : "<<val;
		return val;
	}
	return false;
}


void ToolLauncher::registerNativeMethods()
{
	JNINativeMethod methods[] = {{"saveSessionJavaHelper", "()V", reinterpret_cast<void*>(saveSessionJavaHelper) },
				     {"saveAndStopRunningToolsJNI", "()V", reinterpret_cast<void*>(saveAndStopRunningToolsJNI) },
				     {"saveAndStopRunningInputToolsJNI", "()V", reinterpret_cast<void*>(saveAndStopRunningInputToolsJNI) },
				     {"restoreRunningToolsJNI", "()V", reinterpret_cast<void*>(restoreRunningToolsJNI) },
				     {"nrOfToolsSavedJNI", "()I", reinterpret_cast<void*>(nrOfToolsSavedJNI) },
				     {"nrOfToolsRunningJNI", "()I", reinterpret_cast<void*>(nrOfToolsRunningJNI) },
				     {"hasCtxJNI", "()Z", reinterpret_cast<void*>(hasCtxJNI) },
				    };

	QAndroidJniObject activity = QtAndroid::androidActivity();
	QAndroidJniEnvironment env;
	jclass objectClass = env->GetObjectClass(activity.object<jobject>());

	env->RegisterNatives(objectClass,
						 methods,
						 sizeof(methods) / sizeof(methods[0]));
	env->DeleteLocalRef(objectClass);
}
#endif

bool ToolLauncher::isOpenGlLoaded() const {
	return openGlLoaded;
}

void ToolLauncher::loadOpenGL() {
	// set surfaceFormat as in Qt example: HelloGL2 - https://code.qt.io/cgit/qt/qtbase.git/tree/examples/opengl/hellogl2/main.cpp?h=5.15#n81
	QSurfaceFormat fmt;
	fmt.setDepthBufferSize(24);
	QSurfaceFormat::setDefaultFormat(fmt);

	// This acts as a loader for the OpenGL context, our plots load and draw in the OpenGL context
	// at the same time which causes some race condition and causes the app to hang
	// with this workaround, the app loads the OpenGL context before any plots are created
	// Probably there's a better way to do this
	auto a = new QOpenGLWidget(this);
	delete a;
	openGlLoaded = true;
}
