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

#include <iio.h>

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
	tl_api(new ToolLauncher_API(this)),
	notifier(STDIN_FILENO, QSocketNotifier::Read),
	infoWidget(nullptr),
	calib(nullptr),
	skip_calibration(false),
	calibrating(false),
	debugger_enabled(false),
	indexFile("")
{
	if (!isatty(STDIN_FILENO))
		notifier.setEnabled(false);

	ui->setupUi(this);

	tools << "Oscilloscope" << "Spectrum Analyzer"
	      << "Network Analyzer" << "Signal Generator"
	      << "Logic Analyzer" << "Pattern Generator"
	      << "Digital IO" << "Voltmeter"
	      << "Power Supply" << "Debugger";

	toolIcons << ":/menu/oscilloscope.png"
		<< ":/menu/spectrum_analyzer.png"
		<< ":/menu/network_analyzer.png"
		<< ":/menu/signal_generator.png"
		<< ":/menu/logic_analyzer.png"
		<< ":/menu/pattern_generator.png"
		<< ":/menu/io.png"
		<< ":/menu/voltmeter.png"
		<< ":/menu/power_supply.png"
		<< ":/menu/spectrum_analyzer.png";

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

	const QVector<QString>& uris = searchDevices();
	for (const QString& each : uris)
		addContext(each);

	current = ui->homeWidget;

	ui->menu->setMinimumSize(ui->menu->sizeHint());

	connect(this, SIGNAL(adcCalibrationDone()),
		this, SLOT(enableAdcBasedTools()));
	connect(this, SIGNAL(dacCalibrationDone()),
		this, SLOT(enableDacBasedTools()));
	connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(addRemoteContext()));

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
	connect(ui->btnHome, SIGNAL(toggled(bool)), this,
		SLOT(setButtonBackground(bool)));

	ui->saveBtn->parentWidget()->setEnabled(false);
	ui->loadBtn->parentWidget()->setEnabled(false);

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
	ui->menu->setMinimumSize(ui->menu->sizeHint());
	/* Show a smooth opening when the app starts */
	ui->menu->toggleMenu(true);

	connect(ui->saveBtn, &QPushButton::clicked, this, &ToolLauncher::saveSession);
	connect(ui->loadBtn, &QPushButton::clicked, this, &ToolLauncher::loadSession);

	setupHomepage();
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

void ToolLauncher::loadSession()
{
	if (ctx) {
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
			this->tl_api->load(f.fileName());
			while (!enabledTools.isEmpty()) {
				QPushButton *btn = enabledTools.back();
				btn->setEnabled(true);
				enabledTools.pop();
			}
		}
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

	for (auto it = devices.begin(); it != devices.end();) {
		QString uri = (*it)->second.btn->property("uri").toString();

		if (uri.startsWith("usb:") && !uris.contains(uri)) {
			if ((*it)->second.btn->isChecked()){
				(*it)->second.btn->click();
				return;
			}
			delete *it;
			it = devices.erase(it);
		} else {
			++it;
		}
	}

	for (const QString& uri : uris) {
		if (!uri.startsWith("usb:"))
			continue;

		bool found = false;

		for (const auto each : devices) {
			QString str = each->second.btn->property("uri")
				.toString();

			if (str == uri) {
				found = true;
				break;
			}
		}

		if (!found)
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
	}
}

void ToolLauncher::insertMenuOptions(){
	for (int i = 0; i < position.size(); ++i){
		ui->menuOptionsLayout->insertWidget(i, toolMenu[tools[position[i]]]);
		toolMenu[tools[position[i]]]->setPosition(i);
		ui->buttonGroup_2->addButton(toolMenu[tools[position[i]]]->getToolBtn());
	}

	DragZone *dragZone = new DragZone(ui->menu);
	ui->menuOptionsLayout->addWidget(dragZone);
	connect (dragZone, SIGNAL(requestPositionChange(int, int, bool)), this,
		 SLOT(swapMenuOptions(int, int, bool)));
	connect(dragZone, SIGNAL(highlightLastSeparator(bool)), this,
		SLOT(highlightLast(bool)));

	toolMenu["Debugger"]->setVisible(debugger_enabled);
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
		ui->loadBtn->setToolTip(QString());
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

QPushButton *ToolLauncher::addContext(const QString& uri)
{
	auto pair = new QPair<QWidget, Ui::Device>;
	pair->second.setupUi(&pair->first);

	pair->second.description->setText(uri);

	ui->devicesList->addWidget(&pair->first);

	connect(pair->second.btn, SIGNAL(clicked(bool)),
		this, SLOT(device_btn_clicked(bool)));

	pair->second.btn->setProperty("uri", QVariant(uri));
	devices.append(pair);

	return pair->second.btn;
}

void ToolLauncher::addRemoteContext()
{
	pv::widgets::Popup *popup = new pv::widgets::Popup(ui->homeWidget);
	connect(popup, SIGNAL(closed()), this, SLOT(destroyPopup()));

	QPoint pos = ui->groupBox->mapToGlobal(ui->btnAdd->pos());
	pos += QPoint(ui->btnAdd->width() / 2, ui->btnAdd->height());

	popup->set_position(pos, pv::widgets::Popup::Bottom);
	popup->show();

	ConnectDialog *dialog = new ConnectDialog(popup);
	connect(dialog, &ConnectDialog::newContext,
	[=](const QString& uri) {
		bool found = false;
		for (auto it = devices.begin(); it != devices.end(); ++it) {
			QString dev_uri = (*it)->second.btn->property("uri").toString();
			if (dev_uri == uri) {
				found = true;
				highlightDevice((*it)->second.btn);
				break;
			}
		}
		if (!found) {
			addContext(uri);
		}
		popup->close();
	});
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
	welcome = new QTextBrowser(ui->stackedWidget);
	welcome->setFrameShape(QFrame::NoFrame);
	welcome->setOpenExternalLinks(true);
	welcome->setSource(QUrl("qrc:/scopy.html"));
	ui->stackedWidget->addWidget(welcome);

	// Index page
	index = new QTextBrowser(ui->stackedWidget);
	index->setFrameShape(QFrame::NoFrame);

	if (indexFile == "") {
		return;
	}

	QFileInfo fileInfo(indexFile);
	if (fileInfo.exists()) {
		QFile indexFile(fileInfo.filePath());
		indexFile.open(QFile::ReadOnly);
		if (!indexFile.readAll().isEmpty()) {
			index->setSearchPaths(QStringList(fileInfo.dir().absolutePath()));
			indexFile.close();
			index->setSource(QUrl::fromLocalFile(fileInfo.filePath()));
			ui->stackedWidget->addWidget(index);
			ui->stackedWidget->moveRight();
		} else {
			indexFile.close();
		}
	}
}

void ToolLauncher::swapMenu(QWidget *menu)
{
	Tool *tl = dynamic_cast<Tool* >(menu);
	if (tl){

		MenuOption *mo = static_cast<MenuOption *>(tl->runButton()->parentWidget());
		if (mo->isDetached())
			return;

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
	MenuOption *mo = static_cast<MenuOption *>(btn->parentWidget());

	if (mo->isDetached())
		return;

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

void adiscope::ToolLauncher::resetStylesheets()
{
	setDynamicProperty(ui->btnConnect, "connected", false);
	setDynamicProperty(ui->btnConnect, "failed", false);

	for (auto it = devices.begin(); it != devices.end(); ++it) {
		QPushButton *btn = (*it)->second.btn;
		setDynamicProperty(btn, "connected", false);
		setDynamicProperty(btn, "failed", false);
	}
}

void adiscope::ToolLauncher::device_btn_clicked(bool pressed)
{
	if (pressed) {
		for (auto it = devices.begin(); it != devices.end(); ++it)
			if ((*it)->second.btn != sender()) {
				(*it)->second.btn->setChecked(false);
			}

		if (ui->btnConnect->property("connected").toBool()) {
			ui->btnConnect->click();
		}
	} else {
		disconnect();
	}

	resetStylesheets();
	ui->btnConnect->setEnabled(pressed);
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

		ui->saveBtn->parentWidget()->setEnabled(false);
		ui->loadBtn->parentWidget()->setEnabled(false);

		for (auto x : detachedWindows){
			x->close();
		}

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

void adiscope::ToolLauncher::on_btnConnect_clicked(bool pressed)
{
	if (ctx) {
		disconnect();
		ui->btnConnect->setToolTip(QString("Click to connect the device"));
		return;
	}

	QPushButton *btn = nullptr;
	QLabel *label = nullptr;

	for (auto it = devices.begin(); !btn && it != devices.end(); ++it) {
		if ((*it)->second.btn->isChecked()) {
			btn = (*it)->second.btn;
			label = (*it)->second.name;
		}
	}

	if (!btn) {
		throw std::runtime_error("No enabled device!");
	}

	QString uri = btn->property("uri").toString();

	bool success = switchContext(uri);
	if (success) {
		setDynamicProperty(ui->btnConnect, "connected", true);
		setDynamicProperty(btn, "connected", true);
		search_timer->stop();

		ui->saveBtn->parentWidget()->setEnabled(true);
		ui->loadBtn->parentWidget()->setEnabled(true);

		if (label) {
			label->setText(filter->hw_name());
		}
	} else {
		setDynamicProperty(ui->btnConnect, "failed", true);
		setDynamicProperty(btn, "failed", true);
	}

	Q_EMIT connectionDone(success);
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

	if (filter) {
		delete filter;
		filter = nullptr;
	}

	if (calib) {
		delete calib;
		calib = nullptr;
	}

	if (ctx) {
		iio_context_destroy(ctx);
		ctx = nullptr;
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
		qDebug() << "ERROR: libsigrokdecode init failed.";
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
	}

	if (filter->compatible(TOOL_DMM)) {
		dmm = new DMM(ctx, filter, adc, toolMenu["Voltmeter"]->getToolStopBtn(),
				&js_engine, this);
		adc_users_group.addButton(toolMenu["Voltmeter"]->getToolStopBtn());\
		toolList.push_back(dmm);
	}

	if (filter->compatible(TOOL_DEBUGGER)) {
		debugger = new Debugger(ctx, filter,toolMenu["Debugger"]->getToolStopBtn(),
				&js_engine, this);
		adc_users_group.addButton(toolMenu["Debugger"]->getToolStopBtn());
		QObject::connect(debugger, &Debugger::newDebuggerInstance, this,
				 &ToolLauncher::addDebugWindow);
	}

	if (filter->compatible(TOOL_SPECTRUM_ANALYZER)) {
		spectrum_analyzer = new SpectrumAnalyzer(ctx, filter, adc,
			toolMenu["Spectrum Analyzer"]->getToolStopBtn(),&js_engine, this);
		toolList.push_back(spectrum_analyzer);
		adc_users_group.addButton(toolMenu["Spectrum Analyzer"]->getToolStopBtn());
	}

	if (filter->compatible((TOOL_NETWORK_ANALYZER))) {
		network_analyzer = new NetworkAnalyzer(ctx, filter, adc,
			toolMenu["Network Analyzer"]->getToolStopBtn(), &js_engine, this);
		adc_users_group.addButton(toolMenu["Network Analyzer"]->getToolStopBtn());
		toolList.push_back(network_analyzer);
	}

	Q_EMIT adcToolsCreated();
}


void adiscope::ToolLauncher::enableDacBasedTools()
{
	if (filter->compatible(TOOL_SIGNAL_GENERATOR)) {
		signal_generator = new SignalGenerator(ctx, dacs, filter,
			toolMenu["Signal Generator"]->getToolStopBtn(), &js_engine, this);
		toolList.push_back(signal_generator);
	}
	Q_EMIT dacToolsCreated();
}

bool adiscope::ToolLauncher::switchContext(const QString& uri)
{
	destroyContext();

	if (uri.startsWith("ip:")) {
		previousIp = uri.mid(3);
	}

	ctx = iio_create_context_from_uri(uri.toStdString().c_str());

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
		bool success = loadDecoders(QCoreApplication::applicationDirPath() +
					"/decoders");

		if (!success) {
			search_timer->stop();

			QMessageBox error(this);
			error.setText("There was a problem initializing libsigrokdecode. Some features may be missing");
			error.exec();
		}
	}

	if (filter->compatible(TOOL_DIGITALIO)) {
		dio = new DigitalIO(ctx, filter, toolMenu["Digital IO"]->getToolStopBtn(),
				dioManager, &js_engine, this);
		toolList.push_back(dio);
	}


	if (filter->compatible(TOOL_POWER_CONTROLLER)) {
		power_control = new PowerController(ctx, toolMenu["Power Supply"]->getToolStopBtn(),
				&js_engine, this);
		toolList.push_back(power_control);
	}

	if (filter->compatible(TOOL_LOGIC_ANALYZER)) {
		logic_analyzer = new LogicAnalyzer(ctx, filter, toolMenu["Logic Analyzer"]->getToolStopBtn(),
				&js_engine, this);
		toolList.push_back(logic_analyzer);
	}


	if (filter->compatible((TOOL_PATTERN_GENERATOR))) {
		pattern_generator = new PatternGenerator(ctx, filter,
				toolMenu["Pattern Generator"]->getToolStopBtn(), &js_engine,dioManager, this);
		toolList.push_back(pattern_generator);
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
		for (auto it = devices.begin(); it != devices.end(); ++it) {
			QString dev_uri = (*it)->second.btn->property("uri").toString();
			if (dev_uri == uri) {
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
	if (mo->isDetached() && detached){
		for (auto x : detachedWindows)
			if (x->windowTitle().contains(tool->getName())){
				x->showWindow();
				return;
			}
	}
	mo->setDetached(detached);

	if (detached) {
		/* Switch back to the home screen */
		if (current == static_cast<QWidget *>(tool))
			ui->btnHome->click();

		setDynamicProperty(tool->runButton()->parentWidget(), "selected", false);

		DetachedWindow *window = new DetachedWindow(this->windowIcon());
		window->setCentralWidget(tool);
		window->setWindowTitle("Scopy - " + tool->getName());
		window->resize(sizeHint());
		window->show();
		detachedWindows.push_back(window);

		connect(window, &DetachedWindow::closed, [=](){
			tool->attached();
			detachedWindows.removeOne(window);
		});
		connect(mo->getToolBtn(), &QPushButton::clicked,
			[=](){
			if (detachedWindows.contains(window)){
				window->showWindow();
			}
		});
	}

	tool->setVisible(detached);
}

void ToolLauncher::closeEvent(QCloseEvent *event)
{
	for (auto x : detachedWindows){
		x->close();
	}
	detachedWindows.clear();

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
	int menuSize = ui->menuOptionsLayout->count();

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
			else
				spectrum_analyzer->detached();
		
			}
}

QList<QString> ToolLauncher_API::order()
{
	return tl->getOrder();
}

void ToolLauncher_API::setOrder(QList<QString> list)
{
	tl->setOrder(list);
}

QString ToolLauncher_API::getIndexFile() const
{
	return tl->indexFile;
}

void ToolLauncher_API::setIndexFile(const QString &indexFile)
{
	tl->indexFile = indexFile;
}

bool ToolLauncher_API::menu_opened() const
{
	return tl->ui->btnMenu->isChecked();
}

void ToolLauncher_API::open_menu(bool open)
{
	tl->ui->btnMenu->setChecked(open);
}

bool ToolLauncher_API::hidden() const
{
	return !tl->isVisible();
}

void ToolLauncher_API::hide(bool hide)
{
	tl->setVisible(!hide);
}

void ToolLauncher_API::skip_calibration(bool skip)
{
	tl->skip_calibration = skip;
}

bool ToolLauncher_API::debugger_enabled()
{
	return tl->debugger_enabled;
}

void ToolLauncher_API::enable_debugger(bool enabled)
{
	tl->debugger_enabled = enabled;
}

bool ToolLauncher_API::calibration_skipped()
{
	return tl->skip_calibration;
}

QList<QString> ToolLauncher_API::usb_uri_list()
{
	QList<QString> uri_list;
	auto list = tl->searchDevices();

	for (int i = 0; i < list.size(); i++) {
		uri_list.push_back(list[i]);
	}

	return uri_list;
}

bool ToolLauncher_API::connect(const QString& uri)
{
	QPushButton *btn = nullptr;
	bool did_connect = false;
	bool done = false;

	for (auto it = tl->devices.begin();
	     !btn && it != tl->devices.end(); ++it) {
		QPushButton *tmp = (*it)->second.btn;

		if (tmp->property("uri").toString().compare(uri) == 0) {
			btn = tmp;
		}
	}

	if (!btn) {
		btn = tl->addContext(uri);
	}

	tl->connect(tl, &ToolLauncher::connectionDone,
			[&](bool success) {
		if (!success)
			done = true;
	});

	tl->connect(tl, &ToolLauncher::adcToolsCreated, [&]() {
		did_connect = true;
		done = true;
	});

	btn->click();
	tl->ui->btnConnect->click();

	do {
		QCoreApplication::processEvents();
		QThread::msleep(10);
	} while (!done);
	return did_connect;
}

void ToolLauncher_API::disconnect()
{
	tl->disconnect();
}

void ToolLauncher_API::addIp(const QString& ip)
{
	if (!ip.isEmpty()) {
		QtConcurrent::run(std::bind(&ToolLauncher::checkIp, tl, ip));
	}
}

void ToolLauncher_API::load(const QString& file)
{
	QSettings settings(file, QSettings::IniFormat);

	this->ApiObject::load(settings);

	if (tl->oscilloscope)
		tl->oscilloscope->api->load(settings);
	if (tl->dmm)
		tl->dmm->api->load(settings);
	if (tl->power_control)
		tl->power_control->api->load(settings);
	if (tl->signal_generator)
		tl->signal_generator->api->load(settings);
	if (tl->logic_analyzer)
		tl->logic_analyzer->api->load(settings);
	if (tl->dio)
		tl->dio->api->load(settings);
	if (tl->pattern_generator)
		tl->pattern_generator->api->load(settings);
	if (tl->network_analyzer)
		tl->network_analyzer->api->load(settings);
	if (tl->spectrum_analyzer)
		tl->spectrum_analyzer->api->load(settings);

	for (auto tool : tl->toolList)
		tool->settingsLoaded();
}

void ToolLauncher_API::save(const QString& file)
{
	QSettings settings(file, QSettings::IniFormat);

	this->ApiObject::save(settings);

	if (tl->oscilloscope)
		tl->oscilloscope->api->save(settings);
	if (tl->dmm)
		tl->dmm->api->save(settings);
	if (tl->power_control)
		tl->power_control->api->save(settings);
	if (tl->signal_generator)
		tl->signal_generator->api->save(settings);
	if (tl->logic_analyzer)
		tl->logic_analyzer->api->save(settings);
	if (tl->dio)
		tl->dio->api->save(settings);
	if (tl->pattern_generator)
		tl->pattern_generator->api->save(settings);
	if (tl->network_analyzer)
		tl->network_analyzer->api->save(settings);
	if (tl->spectrum_analyzer)
		tl->spectrum_analyzer->api->save(settings);
}

void ToolLauncher::addDebugWindow()
{
	DetachedWindow *window = new DetachedWindow(this->windowIcon());
	Debugger *debug = new Debugger(ctx, filter,toolMenu["Debugger"]->getToolStopBtn(),
			&js_engine, this);
	QObject::connect(debug, &Debugger::newDebuggerInstance, this,
			 &ToolLauncher::addDebugWindow);

	window->setCentralWidget(debug);
	window->setWindowTitle("Scopy - Debugger");
	window->resize(sizeHint());
	window->show();
	debugWindows.append(window);
	debugInstances.append(debug);
}
