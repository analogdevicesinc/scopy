/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "logging_categories.h"
#include "config.h"
#include "connectDialog.hpp"
#include "dynamicWidget.hpp"
#include "oscilloscope.hpp"
#include "spectrum_analyzer.hpp"
#include "tool_launcher.hpp"
#include "qtjs.hpp"
#include "jsfileio.h"
#include "osc_adc.h"
#include "hw_dac.h"
#include "menuoption.h"
#include "dragzone.h"
#include "debugger.h"
#include "manualcalibration.h"
#include "apiobjectmanager.h"
#include "device_widget.hpp"
#include "user_notes.hpp"
#include "external_script_api.hpp"
#include "animationmanager.h"

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

#include <iio.h>

#include "tool_launcher_api.hpp"

#define TIMER_TIMEOUT_MS 5000
#define ALIVE_TIMER_TIMEOUT_MS 5000

using namespace adiscope;

ToolLauncher::ToolLauncher(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::ToolLauncher), ctx(nullptr),
	power_control(nullptr), dmm(nullptr), signal_generator(nullptr),
	oscilloscope(nullptr), current(nullptr), filter(nullptr),
	logic_analyzer(nullptr), pattern_generator(nullptr), dio(nullptr),
	network_analyzer(nullptr), spectrum_analyzer(nullptr), debugger(nullptr),
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
	m_use_decoders(true)
{
	if (!isatty(STDIN_FILENO))
		notifier.setEnabled(false);

	ui->setupUi(this);

	tools << "Oscilloscope" << "Spectrum Analyzer"
	      << "Network Analyzer" << "Signal Generator"
	      << "Logic Analyzer" << "Pattern Generator"
	      << "Digital IO" << "Voltmeter"
	      << "Power Supply" << "Debugger" << "Calibration";

	toolIcons << ":/menu/oscilloscope.png"
		<< ":/menu/spectrum_analyzer.png"
		<< ":/menu/network_analyzer.png"
		<< ":/menu/signal_generator.png"
		<< ":/menu/logic_analyzer.png"
		<< ":/menu/pattern_generator.png"
		<< ":/menu/io.png"
		<< ":/menu/voltmeter.png"
		<< ":/menu/power_supply.png"
		<< ":/menu/debugger.png"
		<< ":/menu/calibration.png";

	for (int i = 0; i < tools.size(); i++)
		position.push_back(i);

	infoWidget = new InfoWidget(this);
	generateMenu();
	connect(ui->homeWidget, SIGNAL(detachWidget(int)), this,
				SLOT(detachToolOnPosition(int)));
	connect(ui->homeWidget, SIGNAL(changeText(QString)), infoWidget,
		SLOT(setText(QString)));


	setWindowIcon(QIcon(":/icon.ico"));

	// TO DO: remove this when the About menu becomes available
	setWindowTitle(QString("Scopy - ") + QString(SCOPY_VERSION_GIT));

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

	connect(prefPanel, &Preferences::reset, this, &ToolLauncher::resetSession);
	connect(prefPanel, &Preferences::notify, this, &ToolLauncher::readPreferences);

	const QVector<QString>& uris = searchDevices();
	for (const QString& each : uris)
		addContext(each);

	current = ui->homeWidget;

	ui->menu->setMinimumSize(ui->menu->sizeHint());
	devices_btn_group->addButton(ui->btnAdd);
	devices_btn_group->addButton(ui->btnHomepage);

	connect(this, SIGNAL(adcCalibrationDone()),
		this, SLOT(enableAdcBasedTools()));
	connect(this, SIGNAL(dacCalibrationDone()),
		this, SLOT(enableDacBasedTools()));
	connect(ui->btnAdd, SIGNAL(toggled(bool)), this, SLOT(btnAdd_toggled(bool)));
	connect(ui->btnHomepage, SIGNAL(toggled(bool)), this, SLOT(btnHomepage_toggled(bool)));

	tl_api->setObjectName(QString::fromStdString(Filter::tool_name(
			TOOL_LAUNCHER)));
	connect(toolMenu["Oscilloscope"]->getToolBtn(), SIGNAL(clicked()), this,
		SLOT(btnOscilloscope_clicked()));
	connect(toolMenu["Signal Generator"]->getToolBtn(), SIGNAL(clicked()), this,
		SLOT(btnSignalGenerator_clicked()));
	connect(toolMenu["Voltmeter"]->getToolBtn(), SIGNAL(clicked()), this,
		SLOT(btnDMM_clicked()));
	connect(toolMenu["Power Supply"]->getToolBtn(), SIGNAL(clicked()), this,
		SLOT(btnPowerControl_clicked()));
	connect(toolMenu["Logic Analyzer"]->getToolBtn(), SIGNAL(clicked()), this,
		SLOT(btnLogicAnalyzer_clicked()));
	connect(toolMenu["Pattern Generator"]->getToolBtn(), SIGNAL(clicked()), this,
		SLOT(btnPatternGenerator_clicked()));
	connect(toolMenu["Network Analyzer"]->getToolBtn(), SIGNAL(clicked()), this,
		SLOT(btnNetworkAnalyzer_clicked()));
	connect(toolMenu["Digital IO"]->getToolBtn(), SIGNAL(clicked()), this,
		SLOT(btnDigitalIO_clicked()));
	connect(toolMenu["Spectrum Analyzer"]->getToolBtn(), SIGNAL(clicked()), this,
		SLOT(btnSpectrumAnalyzer_clicked()));
	connect(toolMenu["Debugger"]->getToolBtn(), SIGNAL(clicked()), this,
		SLOT(btnDebugger_clicked()));
	connect(toolMenu["Calibration"]->getToolBtn(), SIGNAL(clicked()), this,
		SLOT(btnCalibration_clicked()));


		//option background
	connect(toolMenu["Oscilloscope"]->getToolBtn(), SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));
	connect(toolMenu["Signal Generator"]->getToolBtn(), SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));
	connect(toolMenu["Voltmeter"]->getToolBtn(), SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));
	connect(toolMenu["Power Supply"]->getToolBtn(), SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));
	connect(toolMenu["Logic Analyzer"]->getToolBtn(), SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));
	connect(toolMenu["Pattern Generator"]->getToolBtn(), SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));
	connect(toolMenu["Network Analyzer"]->getToolBtn(), SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));
	connect(toolMenu["Digital IO"]->getToolBtn(), SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));
	connect(toolMenu["Spectrum Analyzer"]->getToolBtn(), SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));
	connect(toolMenu["Debugger"]->getToolBtn(), SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));
	connect(toolMenu["Calibration"]->getToolBtn(), SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));
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
	scopy.copy(tempFile.fileName());
	settings = new QSettings(tempFile.fileName(), QSettings::IniFormat);

	tl_api->ApiObject::load(*settings);

	insertMenuOptions();
	ui->menu->setMinimumSize(ui->menu->sizeHint() + QSize(40, 0));

	/* Show a smooth opening when the app starts */
	ui->menu->toggleMenu(true);

	connect(ui->saveBtn, &QPushButton::clicked, this, &ToolLauncher::saveSession);
	connect(ui->loadBtn, &QPushButton::clicked, this, &ToolLauncher::loadSession);
    
	connect(ui->menu, &MenuAnim::finished, [=](bool opened) {
		for (auto tools : ui->widget->findChildren<MenuOption *>())
			tools->setMaximumWidth(opened ? 500 : 64);
		ui->saveLbl->setVisible(opened);
		ui->loadLbl->setVisible(opened);
		ui->prefBtn->setText(opened ? "Preferences" : "");
	});

	connect(ui->stackedWidget, SIGNAL(moved(int)),
		this, SLOT(pageMoved(int)));


	setupHomepage();
	ui->stackedWidget->setCurrentIndex(0);
	setupAddPage();
	readPreferences();
	ui->stackedWidget->setStyleSheet("background-color:black;");
	this->installEventFilter(this);
	ui->btnConnect->hide();
}

void ToolLauncher::readPreferences()
{
	m_use_decoders = prefPanel->getDigital_decoders_enabled();

	ui->btnNotes->setVisible(prefPanel->getUser_notes_active());
	for (auto tool : toolMenu) {
		tool->enableDoubleClick(prefPanel->getDouble_click_to_detach());
	}
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
		auto export_dialog( new QFileDialog( this ) );
		export_dialog->setWindowModality( Qt::WindowModal );
		export_dialog->setFileMode( QFileDialog::AnyFile );
		export_dialog->setAcceptMode( QFileDialog::AcceptSave );
		export_dialog->setNameFilters({"Scopy-Files (*.ini)"});
		if (export_dialog->exec()){
			QFile f(export_dialog->selectedFiles().at(0));
			this->tl_api->save(f.fileName());
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

void ToolLauncher::loadSession()
{
	auto export_dialog( new QFileDialog( this ) );
	export_dialog->setWindowModality( Qt::WindowModal );
	export_dialog->setFileMode( QFileDialog::AnyFile );
	export_dialog->setAcceptMode( QFileDialog::AcceptOpen );
	export_dialog->setNameFilters({"Scopy-Files (*.ini)"});
	if (export_dialog->exec()){
		QFile f(export_dialog->selectedFiles().at(0));
		QStack<QPushButton*> enabledTools;
		for (auto tool : toolMenu)
			if (tool->getToolStopBtn()->isEnabled()) {
				enabledTools.push(tool->getToolStopBtn());
				tool->getToolStopBtn()->setDisabled(true);
			}
		pathToFile = f.fileName();
		this->tl_api->load(pathToFile);
		while (!enabledTools.isEmpty()) {
			QPushButton *btn = enabledTools.back();
			btn->setEnabled(true);
			enabledTools.pop();
		}
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
	for (int i = 0; i < devices.size(); i++) {
		auto dev = devices.at(i);
		if (dev->isChecked()) {
			return dev;
		}
	}
	return nullptr;
}

int ToolLauncher::getDeviceIndex(DeviceWidget *device)
{
	for (int i = 0; i < devices.size(); i++) {
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

QList<QString> ToolLauncher::getOrder()
{
	QList<QString> list;
	for(int i = 0; i < tools.size(); ++i)
		for (const auto x : toolMenu)
			if (x->getPosition() == i)
				list.push_back(x->getName());
	return list;
}

void ToolLauncher::setOrder(QList<QString> list)
{
	if (tools.size() != list.size())
		return;

	for (int i = 0; i < tools.size(); ++i){
		for (int j = 0; j < list.size(); ++j)
			if (tools[j] == list[i])
				position[i] = j;
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

		bool found = false;

		auto dev = getDevice(uri);

		if (!dev)
			addContext(uri);
	}

	search_timer->start(TIMER_TIMEOUT_MS);
}

void ToolLauncher::generateMenu()
{
	for (int i = 0; i < tools.size(); ++i){
		if (tools[i] == "Oscilloscope" ||
			tools[i] == "Voltmeter" ||
			tools[i] == "Spectrum Analyzer" ||
			tools[i] == "Network Analyzer") {
			toolMenu.insert(tools[i],
					new MenuOption(tools[i],
						toolIcons[i], i, true, ui->menu));
		} else {
		toolMenu.insert(tools[i],
				new MenuOption(tools[i],
						toolIcons[i], i, false, ui->menu));
		}
		connect(toolMenu[tools[i]], SIGNAL(requestPositionChange(int, int, bool)), this,
				SLOT(swapMenuOptions(int, int, bool)));
		connect(toolMenu[tools[i]], SIGNAL(highlight(bool, int)), this,
				SLOT(highlight(bool, int)));
		connect(toolMenu[tools[i]], SIGNAL(enableInfoWidget(bool)), infoWidget,
				SLOT(enable(bool)));
		connect(toolMenu[tools[i]], SIGNAL(changeText(QString)), infoWidget,
				SLOT(setText(QString)));
		connect(toolMenu[tools[i]], SIGNAL(detach(int)), this,
				SLOT(detachToolOnPosition(int)));
	}
}

void ToolLauncher::insertMenuOptions(){

	if (debugger_enabled) {
		for (auto &option : toolMenu) {
			option->setMaxMenuElements(
						option->getMaxMenuElements() + 1);
		}
	}
	if (manual_calibration_enabled) {
		for (auto &option : toolMenu) {
			option->setMaxMenuElements(
						option->getMaxMenuElements() + 1);
		}
	}


	int offset = 0;
	for (int i = 0; i < position.size(); ++i){
		if (toolMenu[tools[position[i]]]->getName() == "Debugger"
				&& !debugger_enabled) {
			offset += 1;
			continue;
		}
		if (toolMenu[tools[position[i]]]->getName() == "Calibration"
				&& !manual_calibration_enabled) {
			offset += 1;
			continue;
		}

		ui->menuOptionsLayout->insertWidget(i - offset, toolMenu[tools[position[i]]]);
		toolMenu[tools[position[i]]]->setPosition(i - offset);
		ui->buttonGroup_2->addButton(toolMenu[tools[position[i]]]->getToolBtn());
	}

	DragZone *dragZone = new DragZone(ui->menu);
	ui->menuOptionsLayout->addWidget(dragZone);
	connect (dragZone, SIGNAL(requestPositionChange(int, int, bool)), this,
		 SLOT(swapMenuOptions(int, int, bool)));
	connect(dragZone, SIGNAL(highlightLastSeparator(bool)), this,
		SLOT(highlightLast(bool)));

	toolMenu["Debugger"]->setVisible(debugger_enabled);
	dragZone->setPosition(ui->menuOptionsLayout->count() - 2);
	toolMenu["Calibration"]->setVisible(manual_calibration_enabled);
}

void ToolLauncher::highlightLast(bool on){
	for (const auto x : toolMenu){
		if (x->getPosition() == tools.size() - 1)
			x->highlightBotSeparator(on);
	}
}

void ToolLauncher::loadToolTips(bool connected){
	if (connected){
		ui->btnHome->setToolTip(QString("Click to open the home menu"));
		toolMenu["Digital IO"]->getToolBtn()->setToolTip(
					QString("Click to open the Digital IO tool"));
		toolMenu["Logic Analyzer"]->getToolBtn()->setToolTip(
					QString("Click to open the Logical Analyzer tool"));
		toolMenu["Network Analyzer"]->getToolBtn()->setToolTip(
					QString("Click to open the Network Analyzer tool"));
		toolMenu["Oscilloscope"]->getToolBtn()->setToolTip(
					QString("Click to open the Oscilloscope tool"));
		toolMenu["Pattern Generator"]->getToolBtn()->setToolTip(
					QString("Click to open the Pattern Generator tool"));
		toolMenu["Power Supply"]->getToolBtn()->setToolTip(
					QString("Click to open the Power Supply tool"));
		toolMenu["Signal Generator"]->getToolBtn()->setToolTip(
					QString("Click to open the Signal Generator tool"));
		toolMenu["Spectrum Analyzer"]->getToolBtn()->setToolTip(
					QString("Click to open the Spectrum Analyzer tool"));
		toolMenu["Voltmeter"]->getToolBtn()->setToolTip(
					QString("Click to open the Voltmeter tool"));
		ui->btnConnect->setToolTip(QString("Click to disconnect the device"));
		ui->saveBtn->setToolTip(QString("Click to save the current session"));
		ui->loadBtn->setToolTip(QString("Click to load a session"));
	} else {
		ui->btnHome->setToolTip(QString());
		toolMenu["Digital IO"]->getToolBtn()->setToolTip(
					QString());
		toolMenu["Logic Analyzer"]->getToolBtn()->setToolTip(
					QString());
		toolMenu["Network Analyzer"]->getToolBtn()->setToolTip(
					QString());
		toolMenu["Oscilloscope"]->getToolBtn()->setToolTip(
					QString());
		toolMenu["Pattern Generator"]->getToolBtn()->setToolTip(
					QString());
		toolMenu["Power Supply"]->getToolBtn()->setToolTip(
					QString());
		toolMenu["Signal Generator"]->getToolBtn()->setToolTip(
					QString());
		toolMenu["Spectrum Analyzer"]->getToolBtn()->setToolTip(
					QString());
		toolMenu["Voltmeter"]->getToolBtn()->setToolTip(
					QString());
		ui->saveBtn->setToolTip(QString());
		ui->loadBtn->setToolTip(QString("Click to load a session"));
		ui->btnConnect->setToolTip(QString("Select a device first"));
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

	tl_api->ApiObject::save(*settings);

	delete settings;
	delete tl_api;
	delete ui;

	saveSettings();
}

void ToolLauncher::destroyPopup()
{
	auto *popup = static_cast<pv::widgets::Popup *>(QObject::sender());

	popup->deleteLater();
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
			connectedDev->connectButton()->setToolTip(QString("Click to connect the device"));
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
			this, SLOT(connectBtn_clicked(bool)));
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
	}
}

void ToolLauncher::btnAdd_toggled(bool toggled)
{
	setDynamicProperty(ui->addWidget, "selected", toggled);
	if (toggled) {
		ui->stackedWidget->slideToIndex(1);
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
	welcome = new QTextBrowser(homepage);
	welcome->setFrameShape(QFrame::NoFrame);
	welcome->setOpenExternalLinks(true);
	welcome->setSource(QUrl("qrc:/scopy.html"));
	layout->addWidget(welcome);

	QWidget *reportRegion = new QWidget(homepage);
	QHBoxLayout *reportLayout = new QHBoxLayout(reportRegion);
	QPushButton *reportBtn = new QPushButton(reportRegion);
	reportBtn->setText("Report a bug!");
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
		std::string os = QSysInfo::prettyProductName().toStdString();
		std::string gittag = SCOPY_VERSION_GIT;
		std::string fw = "";
		if (ctx) {
			fw = std::string(iio_context_get_attr_value(ctx, "fw_version"));
		}
		QSettings settings;
		QFileInfo info(settings.fileName());
		std::string head = "https://github.com/analogdevicesinc/scopy/issues/new?title=%3CInstrument%3E:%20%3CShort%20description%20of%20the%20bug%3E&body=";
		std::string os_version_urlstring = "OS%20Version: " + os;
		std::string fw_version_urlstring = "%0AFW%20Version: " + fw;
		std::string gittag_urlstring = "%0ASW%20Version: " + gittag;
		std::string description_urlstring = "%0A%0ADescription%20of%20the%20bug:%3Cdescription%3E%0ASteps%20to%20reproduce:"
					  "%0A-%0A-%0A%0AThe%20ini%20files%20might%20be%20useful%20to%20reproduce%20the%20error.";
		std::string ini_file_urlstring = "%0AThe%20ini%20file%20is%20located%20at: " + info.absoluteFilePath().toStdString();
		std::string finalpart = "%0APlease%20consider%20attaching%20it.&labels=bug,reported-from-scopy";
		QUrl url(QString::fromStdString(head + os_version_urlstring +
						fw_version_urlstring + gittag_urlstring +
						description_urlstring + ini_file_urlstring +
						finalpart));
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
		bool found = false;
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
	Tool *tl = dynamic_cast<Tool* >(menu);
	if (tl){

		MenuOption *mo = static_cast<MenuOption *>(tl->runButton()->parentWidget());
		if (mo->isDetached()) {
			// Force the tool to come to the foreground
			tl->detached();
			return;
		}

	}

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
	MenuOption *mo = dynamic_cast<MenuOption *>(btn->parentWidget());

	if (mo) {
		if (mo->isDetached()) {
			return;
		}
	}

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


	if (pressed){
		ui->btnConnect->setToolTip(QString("Click to connect the device"));
	} else {
		ui->btnConnect->setToolTip(QString("Select a device first"));
	}
}

void adiscope::ToolLauncher::disconnect()
{
	/* Switch back to home screen */
	ui->btnHome->click();

	if (ctx) {
		if (calibrating) {
			calib->cancelCalibration();
			calibration_thread.waitForFinished();
		}
		auto iio=iio_manager::get_instance(ctx,filter->device_name(TOOL_DMM));
		iio->stop_all();
		alive_timer->stop();
		toolMenu["Digital IO"]->getToolStopBtn()->setChecked(false);
		toolMenu["Logic Analyzer"]->getToolStopBtn()->setChecked(false);
		toolMenu["Network Analyzer"]->getToolStopBtn()->setChecked(false);
		toolMenu["Oscilloscope"]->getToolStopBtn()->setChecked(false);
		toolMenu["Pattern Generator"]->getToolStopBtn()->setChecked(false);
		toolMenu["Power Supply"]->getToolStopBtn()->setChecked(false);
		toolMenu["Signal Generator"]->getToolStopBtn()->setChecked(false);
		toolMenu["Spectrum Analyzer"]->getToolStopBtn()->setChecked(false);
		toolMenu["Voltmeter"]->getToolStopBtn()->setChecked(false);
		toolMenu["Debugger"]->getToolStopBtn()->setChecked(false);
		toolMenu["Calibration"]->getToolStopBtn()->setChecked(false);

		ui->saveBtn->parentWidget()->setEnabled(false);

		destroyContext();
		loadToolTips(false);
		resetStylesheets();
		search_timer->start(TIMER_TIMEOUT_MS);
	}

	/* Update the list of devices now */
	updateListOfDevices(searchDevices());
}

void adiscope::ToolLauncher::ping()
{
	int ret = iio_context_get_version(ctx, nullptr, nullptr, nullptr);

	if (ret < 0)
		disconnect();
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
		selectedDev->connectButton()->setText("Connecting...");
	} else {
		selectedDev->connectButton()->setText("Disconnecting...");
	}
	QApplication::processEvents();

	/* Disconnect connected device, if any */
	if (ctx) {
		connectedDev->setConnected(false, false);
		disconnect();
		connectedDev->connectButton()->setToolTip(QString("Click to connect the device"));
	}

	if (connectedDev != selectedDev) {
		/* Connect to the selected device, if any */
		if (selectedDev) {
			QString uri = selectedDev->uri();
			selectedDev->infoPage()->identifyDevice(false);
			bool success = switchContext(uri);
			if (success) {
				selectedDev->setConnected(true, false, ctx);
				selectedDev->connectButton()->setText("Calibrating...");
				selectedDev->setName(filter->hw_name());
				selectedDev->infoPage()->identifyDevice(true);
				setDynamicProperty(ui->btnConnect, "connected", true);

				search_timer->stop();

				ui->saveBtn->parentWidget()->setEnabled(true);

			} else {
				setDynamicProperty(ui->btnConnect, "failed", true);
				selectedDev->setConnected(false, true);
				selectedDev->connectButton()->setEnabled(true);
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

void adiscope::ToolLauncher::calibrate()
{
	bool ok=true;

	if (!skip_calibration) {
		ok=false;
		calibrating=true;
		auto old_dmm_text = toolMenu["Voltmeter"]->getToolBtn()->text();
		auto old_osc_text = toolMenu["Oscilloscope"]->getToolBtn()->text();
		auto old_siggen_text = toolMenu["Signal Generator"]->getToolBtn()->text();
		auto old_spectrum_text = toolMenu["Spectrum Analyzer"]->getToolBtn()->text();
		auto old_network_text = toolMenu["Network Analyzer"]->getToolBtn()->text();

		toolMenu["Voltmeter"]->getToolBtn()->setText("Calibrating...");
		toolMenu["Oscilloscope"]->getToolBtn()->setText("Calibrating...");
		toolMenu["Signal Generator"]->getToolBtn()->setText("Calibrating...");
		toolMenu["Spectrum Analyzer"]->getToolBtn()->setText("Calibrating...");
		toolMenu["Network Analyzer"]->getToolBtn()->setText("Calibrating...");

		if (calib->isInitialized()) {
			calib->setHardwareInCalibMode();
			ok = calib->calibrateAll();
			calib->restoreHardwareFromCalibMode();
		}

		toolMenu["Voltmeter"]->getToolBtn()->setText(old_dmm_text);
		toolMenu["Oscilloscope"]->getToolBtn()->setText(old_osc_text);
		toolMenu["Signal Generator"]->getToolBtn()->setText(old_siggen_text);
		toolMenu["Spectrum Analyzer"]->getToolBtn()->setText(old_spectrum_text);
		toolMenu["Network Analyzer"]->getToolBtn()->setText(old_network_text);
	}

	calibrating=false;

	if (ok) {
		Q_EMIT adcCalibrationDone();
		Q_EMIT dacCalibrationDone();
	}
}

void adiscope::ToolLauncher::enableAdcBasedTools()
{
	if (filter->compatible(TOOL_OSCILLOSCOPE)) {
		oscilloscope = new Oscilloscope(ctx, filter, adc,
						toolMenu["Oscilloscope"]->getToolStopBtn(),
						&js_engine, this);
		toolList.push_back(oscilloscope);
		adc_users_group.addButton(toolMenu["Oscilloscope"]->getToolStopBtn());
		connect(oscilloscope, &Oscilloscope::showTool, [=]() {
			toolMenu["Oscilloscope"]->getToolBtn()->click();
		});
	}

	if (filter->compatible(TOOL_DMM)) {
		dmm = new DMM(ctx, filter, adc, toolMenu["Voltmeter"]->getToolStopBtn(),
				&js_engine, this);
		adc_users_group.addButton(toolMenu["Voltmeter"]->getToolStopBtn());\
		toolList.push_back(dmm);
		connect(dmm, &DMM::showTool, [=]() {
			toolMenu["Voltmeter"]->getToolBtn()->click();
		});
	}

	if (filter->compatible(TOOL_DEBUGGER)) {
		debugger = new Debugger(ctx, filter,toolMenu["Debugger"]->getToolStopBtn(),
				&js_engine, this);
		adc_users_group.addButton(toolMenu["Debugger"]->getToolStopBtn());
		QObject::connect(debugger, &Debugger::newDebuggerInstance, this,
				 &ToolLauncher::addDebugWindow);
	}

	if (filter->compatible(TOOL_CALIBRATION)) {
		manual_calibration = new ManualCalibration(ctx, filter,toolMenu["Calibration"]->getToolStopBtn(),
				&js_engine, this, calib);
		adc_users_group.addButton(toolMenu["Calibration"]->getToolStopBtn());
		toolList.push_back(manual_calibration);
	}

	if (filter->compatible(TOOL_SPECTRUM_ANALYZER)) {
		spectrum_analyzer = new SpectrumAnalyzer(ctx, filter, adc,
			toolMenu["Spectrum Analyzer"]->getToolStopBtn(),&js_engine, this);
		toolList.push_back(spectrum_analyzer);
		adc_users_group.addButton(toolMenu["Spectrum Analyzer"]->getToolStopBtn());
		connect(spectrum_analyzer, &SpectrumAnalyzer::showTool, [=]() {
			toolMenu["Spectrum Analyzer"]->getToolBtn()->click();
		});
	}

	if (filter->compatible((TOOL_NETWORK_ANALYZER))) {
		network_analyzer = new NetworkAnalyzer(ctx, filter, adc,
			toolMenu["Network Analyzer"]->getToolStopBtn(), &js_engine, this);
		adc_users_group.addButton(toolMenu["Network Analyzer"]->getToolStopBtn());
		toolList.push_back(network_analyzer);
		connect(network_analyzer, &NetworkAnalyzer::showTool, [=]() {
			toolMenu["Network Analyzer"]->getToolBtn()->click();
		});
	}

	Q_EMIT adcToolsCreated();
}


void adiscope::ToolLauncher::enableDacBasedTools()
{
	if (filter->compatible(TOOL_SIGNAL_GENERATOR)) {
		signal_generator = new SignalGenerator(ctx, dacs, filter,
			toolMenu["Signal Generator"]->getToolStopBtn(), &js_engine, this);
		toolList.push_back(signal_generator);
		connect(signal_generator, &SignalGenerator::showTool, [=]() {
			toolMenu["Signal Generator"]->getToolBtn()->click();
		});
	}
	if (pathToFile != "") {
		this->tl_api->load(pathToFile);
	}

	Q_EMIT dacToolsCreated();
	selectedDev->connectButton()->setText("Disconnect");
	selectedDev->connectButton()->setEnabled(true);
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

	alive_timer->start(ALIVE_TIMER_TIMEOUT_MS);

	filter = new Filter(ctx);

	dacs.clear();

	// Find available DACs
	QList<struct iio_device *> iio_dacs;
	for (unsigned int dev_id = 0; ; dev_id++) {
		struct iio_device *dev;
		try {
			dev = filter->find_device(ctx,
					TOOL_SIGNAL_GENERATOR, dev_id);
		} catch (std::exception &ex) {
			break;
		}
		iio_dacs.push_back(dev);
	}

	if (filter->hw_name().compare("M2K") == 0) {
		adc = AdcBuilder::newAdc(AdcBuilder::M2K, ctx,
			filter->find_device(ctx, TOOL_OSCILLOSCOPE));

		for (int i = 0; i < iio_dacs.size(); i++) {
			auto dac = DacBuilder::newDac(DacBuilder::M2K, ctx,
				iio_dacs[i]);
			dacs.push_back(dac);
		}
	} else {
		adc = AdcBuilder::newAdc(AdcBuilder::GENERIC, ctx,
			filter->find_device(ctx, TOOL_OSCILLOSCOPE));
		for (int i = 0; i < iio_dacs.size(); i++) {
			auto dac = DacBuilder::newDac(DacBuilder::GENERIC, ctx,
				iio_dacs[i]);
			dacs.push_back(dac);
		}
	}

	auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc);
	std::shared_ptr<M2kDac> m2k_dac_a;
	std::shared_ptr<M2kDac> m2k_dac_b;
	for(int i = 0; i < dacs.size(); i++) {
		if(i == 0) {
			m2k_dac_a = std::dynamic_pointer_cast<M2kDac>(dacs.at(i));
		}
		if(i == 1 ){
			m2k_dac_b = std::dynamic_pointer_cast<M2kDac>(dacs.at(i));
		}
	}
	calib = new Calibration(ctx, &js_engine, m2k_adc, m2k_dac_a, m2k_dac_b);
	calib->initialize();


	if (filter->compatible(TOOL_PATTERN_GENERATOR)
	    || filter->compatible(TOOL_DIGITALIO)) {
		dioManager = new DIOManager(ctx,filter);

	}

	if (filter->compatible(TOOL_LOGIC_ANALYZER)
	    || filter->compatible(TOOL_PATTERN_GENERATOR)) {

		if (!m_use_decoders) {
			search_timer->stop();

			QMessageBox info(this);
			info.setText("Digital decoders support is disabled. Some features may be missing");
			info.exec();
		} else {
			bool success = loadDecoders(QCoreApplication::applicationDirPath() +
						    "/decoders");

			if (!success) {
				search_timer->stop();

				QMessageBox error(this);
				error.setText("There was a problem initializing libsigrokdecode. Some features may be missing");
				error.exec();
			}
		}
	}

	if (filter->compatible(TOOL_DIGITALIO)) {
		dio = new DigitalIO(ctx, filter, toolMenu["Digital IO"]->getToolStopBtn(),
				dioManager, &js_engine, this);
		toolList.push_back(dio);
		connect(dio, &DigitalIO::showTool, [=]() {
			toolMenu["Digital IO"]->getToolBtn()->click();
		});
	}


	if (filter->compatible(TOOL_POWER_CONTROLLER)) {
		power_control = new PowerController(ctx, toolMenu["Power Supply"]->getToolStopBtn(),
				&js_engine, this);
		toolList.push_back(power_control);
		connect(power_control, &PowerController::showTool, [=]() {
			toolMenu["Power Supply"]->getToolBtn()->click();
		});
	}

	if (filter->compatible(TOOL_LOGIC_ANALYZER)) {
		logic_analyzer = new LogicAnalyzer(ctx, filter, toolMenu["Logic Analyzer"]->getToolStopBtn(),
				&js_engine, this);
		toolList.push_back(logic_analyzer);
		connect(logic_analyzer, &LogicAnalyzer::showTool, [=]() {
			toolMenu["Logic Analyzer"]->getToolBtn()->click();
		});
	}


	if (filter->compatible((TOOL_PATTERN_GENERATOR))) {
		pattern_generator = new PatternGenerator(ctx, filter,
				toolMenu["Pattern Generator"]->getToolStopBtn(), &js_engine,dioManager, this);
		toolList.push_back(pattern_generator);
		connect(pattern_generator, &PatternGenerator::showTool, [=]() {
			toolMenu["Pattern Generator"]->getToolBtn()->click();
		});
	}

	connect(toolMenu["Network Analyzer"]->getToolStopBtn(),
			&QPushButton::toggled,
			[=](bool en) {
		if(en) {
			if(!toolMenu["Signal Generator"]->getToolStopBtn()->isChecked())
				return;
			toolMenu["Signal Generator"]->getToolStopBtn()->setChecked(false);
		}
	});
	connect(toolMenu["Signal Generator"]->getToolStopBtn(),
			&QPushButton::toggled,
			[=](bool en) {
		if(en) {
			if(adc_users_group.checkedId() == adc_users_group.id(toolMenu["Network Analyzer"]->getToolStopBtn())){
				auto btn = dynamic_cast<CustomPushButton*>(
							toolMenu["Network Analyzer"]->getToolStopBtn());
				btn->setChecked(false);
			}
		}
	});

	loadToolTips(true);
	calibration_thread = QtConcurrent::run(std::bind(&ToolLauncher::calibrate,
					       this));

	return true;
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
			out << "Exception:" << val.toString() << endl;
		} else if (!val.isUndefined()) {
			out << val.toString() << endl;
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

	MenuOption *mo = static_cast<MenuOption *>(tool->runButton()->parentWidget());

	mo->setDetached(detached);

	if (detached) {
		/* Switch back to the home screen */
		if (current == static_cast<QWidget *>(tool))
			ui->btnHome->click();

		setDynamicProperty(tool->runButton()->parentWidget(), "selected", false);
	}

	if (detached) {
		ui->buttonGroup_2->removeButton(mo->getToolBtn());
	} else {
		ui->buttonGroup_2->addButton(mo->getToolBtn());
	}

	tool->setVisible(detached);
}

void ToolLauncher::closeEvent(QCloseEvent *event)
{
	// Notify tools that the launcher is closing
	Q_EMIT launcherClosed();

	// Close all detached windows
	QApplication::closeAllWindows();

	for (auto iterator : debugInstances) {
		delete iterator;
	}
	debugInstances.clear();

	for (auto iterator : debugWindows) {
		iterator->close();
		delete iterator;
	}
	debugWindows.clear();
}


void ToolLauncher::swapMenuOptions(int source, int destination, bool dropAfter)
{
	int menuSize = toolMenu["Digital IO"]->getMaxMenuElements() + 1;

	QWidget *sourceWidget = ui->menuOptionsLayout->itemAt(source)->widget();
	if (dropAfter == true){
		for (int i = source + 1; i < destination + 1; i++ ){
			QWidget *dest = ui->menuOptionsLayout->itemAt(i)->widget();
			UpdatePosition(dest, i - 1);
		}
		UpdatePosition(sourceWidget, destination);
		ui->menuOptionsLayout->removeWidget(sourceWidget);
		ui->menuOptionsLayout->insertWidget(destination, sourceWidget);
		return;
	}
	if (destination == menuSize - 1 && source != menuSize - 2){
		for (int i = source + 1; i < menuSize - 1; i++ ){
			QWidget *dest = ui->menuOptionsLayout->itemAt(i)->widget();
			UpdatePosition(dest, i - 1);
		}
		UpdatePosition(sourceWidget, destination - 1);
		ui->menuOptionsLayout->removeWidget(sourceWidget);
		ui->menuOptionsLayout->insertWidget(destination - 1, sourceWidget);
		return;
	}
	if (destination == 0){
		for (int i = 0; i < source; i++ ){
			QWidget *dest = ui->menuOptionsLayout->itemAt(i)->widget();
			UpdatePosition(dest, i + 1);
		}
		UpdatePosition(sourceWidget, destination);
		ui->menuOptionsLayout->removeWidget(sourceWidget);
		ui->menuOptionsLayout->insertWidget(destination, sourceWidget);
		return;
	}
	if (source < destination){
		for (int i = source + 1; i < destination; i++ ){
			QWidget *dest = ui->menuOptionsLayout->itemAt(i)->widget();
			UpdatePosition(dest, i - 1);
		}
		UpdatePosition(sourceWidget, destination - 1);
		ui->menuOptionsLayout->removeWidget(sourceWidget);
		ui->menuOptionsLayout->insertWidget(destination - 1, sourceWidget);
		return;
	} else {
		for (int i = destination; i < source; i++ ){
			QWidget *dest = ui->menuOptionsLayout->itemAt(i)->widget();
			UpdatePosition(dest, i + 1);
		}
	}
	UpdatePosition(sourceWidget, destination);
	ui->menuOptionsLayout->removeWidget(sourceWidget);
	ui->menuOptionsLayout->insertWidget(destination, sourceWidget);
}

void ToolLauncher::highlight(bool on, int position)
{
	for (const auto x: toolMenu)
		if (x->getPosition() == position && !x->isVisible())
			position++;

	for (const auto x : toolMenu)
		if (x->getPosition() == position){
			x->highlightNeighbour(on);
			return;
		}
}

Preferences *ToolLauncher::getPrefPanel() const
{
	return prefPanel;
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
	return QObject::eventFilter(watched, event);
}

void ToolLauncher::UpdatePosition(QWidget *widget, int position){
	MenuOption *menuOption = static_cast<MenuOption *>(widget);
	menuOption->setPosition(position);
}

void ToolLauncher::detachToolOnPosition(int position)
{
	for (const auto x : toolMenu)
		if (x->getPosition() == position){
			if (x->getName() == "Oscilloscope")
				oscilloscope->detached();
			else if (x->getName() == "Digital IO")
				dio->detached();
			else if (x->getName() == "Voltmeter")
				dmm->detached();
			else if (x->getName() == "Power Supply")
				power_control->detached();
			else if (x->getName() == "Signal Generator")
				signal_generator->detached();
			else if (x->getName() == "Pattern Generator")
				pattern_generator->detached();
			else if (x->getName() == "Logic Analyzer")
				logic_analyzer->detached();
			else if (x->getName() == "Network Analyzer")
				network_analyzer->detached();
			else if (x->getName() == "Debugger")
				debugger->detached();
			else if (x->getName() == "Calibration")
				manual_calibration->detached();
			else
				spectrum_analyzer->detached();
		
			}
}
