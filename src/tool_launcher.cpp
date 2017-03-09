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
#include "tool_launcher.hpp"

#include "ui_device.h"
#include "ui_tool_launcher.h"

#include <QDebug>
#include <QtConcurrentRun>
#include <QSignalTransition>

#include <iio.h>

using namespace adiscope;

ToolLauncher::ToolLauncher(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::ToolLauncher), ctx(nullptr),
	power_control(nullptr), dmm(nullptr), signal_generator(nullptr),
	oscilloscope(nullptr), current(nullptr), filter(nullptr),
	logic_analyzer(nullptr), pattern_generator(nullptr),
	network_analyzer(nullptr), tl_api(new ToolLauncher_API(this)),
	notifier(STDIN_FILENO, QSocketNotifier::Read)
{
	struct iio_context_info **info;
	unsigned int nb_contexts;

	ui->setupUi(this);

	setWindowIcon(QIcon(":/icon.ico"));

	// TO DO: remove this when the About menu becomes available
	setWindowTitle(QString("Scopy - ") + QString(SCOPY_VERSION_GIT));

	struct iio_scan_context *scan_ctx = iio_create_scan_context("usb", 0);
	if (!scan_ctx) {
		std::cerr << "Unable to create scan context!" << std::endl;
		return;
	}

	ssize_t ret = iio_scan_context_get_info_list(scan_ctx, &info);
	if (ret < 0) {
		std::cerr << "Unable to scan!" << std::endl;
		return;
	}

	nb_contexts = static_cast<unsigned int>(ret);

	for (unsigned int i = 0; i < nb_contexts; i++) {
		const char *uri = iio_context_info_get_uri(info[i]);

		if (!QString(uri).startsWith("usb:"))
			continue;

		addContext(QString(uri));
	}

	iio_context_info_list_free(info);
	iio_scan_context_destroy(scan_ctx);

	current = ui->homeWidget;

	ui->menu->setMinimumSize(ui->menu->sizeHint());

	connect(this, SIGNAL(calibrationDone(float, float)),
			this, SLOT(enableCalibTools(float, float)));
	connect(this, SIGNAL(dacCalibrationDone(float, float)),
			this, SLOT(enableDacBasedTools(float, float)));
	connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(addRemoteContext()));

	tl_api->load();

	/* Show a smooth opening when the app starts */
	ui->menu->toggleMenu(true);

	connect(ui->btnOscilloscope, SIGNAL(toggled(bool)), this,
			SLOT(setButtonBackground(bool)));
	connect(ui->btnSignalGenerator, SIGNAL(toggled(bool)), this,
			SLOT(setButtonBackground(bool)));
	connect(ui->btnDMM, SIGNAL(toggled(bool)), this,
			SLOT(setButtonBackground(bool)));
	connect(ui->btnPowerControl, SIGNAL(toggled(bool)), this,
			SLOT(setButtonBackground(bool)));
	connect(ui->btnLogicAnalyzer, SIGNAL(toggled(bool)), this,
			SLOT(setButtonBackground(bool)));
	connect(ui->btnPatternGenerator, SIGNAL(toggled(bool)), this,
			SLOT(setButtonBackground(bool)));
	connect(ui->btnNetworkAnalyzer, SIGNAL(toggled(bool)), this,
			SLOT(setButtonBackground(bool)));

	js_engine.installExtensions(QJSEngine::ConsoleExtension);
	tl_api->js_register(&js_engine);

	connect(&notifier, SIGNAL(activated(int)), this, SLOT(hasText()));
}

ToolLauncher::~ToolLauncher()
{
	destroyContext();

	for (auto it = devices.begin(); it != devices.end(); ++it)
		delete *it;
	devices.clear();

	tl_api->save();
	delete tl_api;
	delete ui;
}

void ToolLauncher::destroyPopup()
{
	auto *popup = static_cast<pv::widgets::Popup *>(QObject::sender());

	popup->deleteLater();
}

QPushButton * ToolLauncher::addContext(const QString& uri)
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
				addContext(uri);
				popup->close();
			});
}

void ToolLauncher::swapMenu(QWidget *menu)
{
	current->setVisible(false);
	ui->centralLayout->removeWidget(current);

	current = menu;

	ui->centralLayout->addWidget(current);
	current->setVisible(true);
}

void ToolLauncher::setButtonBackground(bool on)
{
	auto *btn = static_cast<QPushButton *>(QObject::sender());

	setDynamicProperty(btn->parentWidget(), "selected", on);
}

void ToolLauncher::on_btnOscilloscope_clicked()
{
	swapMenu(static_cast<QWidget *>(oscilloscope));
}

void ToolLauncher::on_btnSignalGenerator_clicked()
{
	swapMenu(static_cast<QWidget *>(signal_generator));
}

void ToolLauncher::on_btnDMM_clicked()
{
	swapMenu(static_cast<QWidget *>(dmm));
}

void ToolLauncher::on_btnPowerControl_clicked()
{
	swapMenu(static_cast<QWidget *>(power_control));
}

void ToolLauncher::on_btnLogicAnalyzer_clicked()
{
	swapMenu(static_cast<QWidget *>(logic_analyzer));
}

void adiscope::ToolLauncher::on_btnPatternGenerator_clicked()
{
	swapMenu(static_cast<QWidget *>(pattern_generator));
}

void adiscope::ToolLauncher::on_btnNetworkAnalyzer_clicked()
{
	swapMenu(static_cast<QWidget *>(network_analyzer));
}

void adiscope::ToolLauncher::apply_m2k_fixes(struct iio_context *ctx)
{
	struct iio_device *dev = iio_context_find_device(ctx, "ad9963");

	/* Configure TX path */
	iio_device_reg_write(dev, 0x68, 0x1B);  // IGAIN1 +-6db  0.25db steps
	iio_device_reg_write(dev, 0x6B, 0x1B);  //
	iio_device_reg_write(dev, 0x69, 0x1C);  // IGAIN2 +-2.5%
	iio_device_reg_write(dev, 0x6C, 0x1C);
	iio_device_reg_write(dev, 0x6A, 0x20);  // IRSET +-20%
	iio_device_reg_write(dev, 0x6D, 0x20);
}

void adiscope::ToolLauncher::on_btnHome_clicked()
{
	swapMenu(ui->homeWidget);
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
			if ((*it)->second.btn != sender())
				(*it)->second.btn->setChecked(false);

		if (ui->btnConnect->property("connected").toBool())
			ui->btnConnect->click();
	} else {
		destroyContext();
	}

	resetStylesheets();
	ui->btnConnect->setEnabled(pressed);
}

void adiscope::ToolLauncher::on_btnConnect_clicked(bool pressed)
{
	if (ctx) {
		destroyContext();
		resetStylesheets();
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

	if (!btn)
		throw std::runtime_error("No enabled device!");

	QString uri = btn->property("uri").toString();

	if (switchContext(uri)) {
		setDynamicProperty(ui->btnConnect, "connected", true);
		setDynamicProperty(btn, "connected", true);

		if (label)
			label->setText(filter->hw_name());
	} else {
		setDynamicProperty(ui->btnConnect, "failed", true);
		setDynamicProperty(btn, "failed", true);
	}
}

void adiscope::ToolLauncher::destroyContext()
{
	ui->oscilloscope->setDisabled(true);
	ui->signalGenerator->setDisabled(true);
	ui->dmm->setDisabled(true);
	ui->powerControl->setDisabled(true);
	ui->logicAnalyzer->setDisabled(true);
	ui->patternGenerator->setDisabled(true);
	ui->networkAnalyzer->setDisabled(true);

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

	if(logic_analyzer) {
		delete logic_analyzer;
		logic_analyzer = nullptr;
	}

	if(pattern_generator) {
		delete pattern_generator;
		pattern_generator = nullptr;
	}

	if(network_analyzer) {
		delete network_analyzer;
		network_analyzer = nullptr;
	}

	if (filter) {
		delete filter;
		filter = nullptr;
	}

	if (ctx) {
		iio_context_destroy(ctx);
		ctx = nullptr;
	}
}

void adiscope::ToolLauncher::calibrate()
{
	auto old_dmm_text = ui->btnDMM->text();
	auto old_osc_text = ui->btnOscilloscope->text();
	auto old_siggen_text = ui->btnSignalGenerator->text();

	ui->btnDMM->setText("Calibrating...");
	ui->btnOscilloscope->setText("Calibrating...");
	ui->btnSignalGenerator->setText("Calibrating...");

	Calibration calib(ctx);

	calib.initialize();
	calib.calibrateAll();
	calib.restoreTriggerSetup();

	ui->btnDMM->setText(old_dmm_text);
	ui->btnOscilloscope->setText(old_osc_text);
	ui->btnSignalGenerator->setText(old_siggen_text);

	Q_EMIT calibrationDone(calib.adcGainChannel0(), calib.adcGainChannel1());
	Q_EMIT dacCalibrationDone(calib.dacAvlsb(), calib.dacBvlsb());
}

void adiscope::ToolLauncher::enableCalibTools(float gain_ch1, float gain_ch2)
{
	if (filter->compatible(TOOL_OSCILLOSCOPE)) {
		oscilloscope = new Oscilloscope(ctx, filter,
				ui->stopOscilloscope, &js_engine,
				gain_ch1, gain_ch2, this);

		ui->oscilloscope->setEnabled(true);
	}

	if (filter->compatible(TOOL_DMM)) {
		dmm = new DMM(ctx, filter, ui->stopDMM, &js_engine,
				gain_ch1, gain_ch2, this);
		dmm->setVisible(false);
		ui->dmm->setEnabled(true);
	}
}

void adiscope::ToolLauncher::enableDacBasedTools(float dacA_vlsb,
	float dacB_vlsb)
{
	if (filter->compatible(TOOL_SIGNAL_GENERATOR)) {
		signal_generator = new SignalGenerator(ctx, filter,
				ui->stopSignalGenerator, &js_engine, this);

		struct iio_device *dev = iio_context_find_device(ctx, "m2k-dac-a");
		if (dev) {
			struct iio_channel *chn = iio_device_find_channel(dev,
					"voltage0", true);
			if (chn)
				signal_generator->set_vlsb_of_channel(
					iio_channel_get_id(chn),
					iio_device_get_name(dev), dacA_vlsb);
		}

		dev = iio_context_find_device(ctx, "m2k-dac-b");
		if (dev) {
			struct iio_channel *chn = iio_device_find_channel(dev,
					"voltage0", true);
			if (chn)
				signal_generator->set_vlsb_of_channel(
					iio_channel_get_id(chn),
					iio_device_get_name(dev), dacB_vlsb);
		}

		signal_generator->setVisible(false);
		ui->signalGenerator->setEnabled(true);
	}
}

bool adiscope::ToolLauncher::switchContext(const QString &uri)
{
	destroyContext();

	if (uri.startsWith("ip:"))
		previousIp = uri.mid(3);

	ctx = iio_create_context_from_uri(uri.toStdString().c_str());
	if (!ctx)
		return false;

	filter = new Filter(ctx);

	if (filter->hw_name().compare("M2K") == 0)
		apply_m2k_fixes(ctx);


	if (filter->compatible(TOOL_POWER_CONTROLLER)) {
		power_control = new PowerController(ctx,
				ui->stopPowerControl, &js_engine, this);
		power_control->setVisible(false);
		ui->powerControl->setEnabled(true);
	}


	if (filter->compatible(TOOL_LOGIC_ANALYZER)) {
		logic_analyzer = new LogicAnalyzer(ctx, filter,
				ui->stopLogicAnalyzer, &js_engine, this);
		logic_analyzer->setVisible(false);
		ui->logicAnalyzer->setEnabled(true);
	}


	if (filter->compatible((TOOL_PATTERN_GENERATOR))) {
		pattern_generator = new PatternGenerator (ctx, filter,
				ui->stopPatternGenerator, &js_engine, this);
		pattern_generator->setVisible(false);
		ui->patternGenerator->setEnabled(true);
	}


	if (filter->compatible((TOOL_NETWORK_ANALYZER))) {
		network_analyzer = new NetworkAnalyzer(ctx, filter,
				ui->stopNetworkAnalyzer, &js_engine, this);
		network_analyzer->setVisible(false);
		ui->networkAnalyzer->setEnabled(true);
	}


	QtConcurrent::run(std::bind(&ToolLauncher::calibrate, this));

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
		if (val.isError())
			out << "Exception:" << val.toString() << endl;
		else if (!val.isUndefined())
			out << val.toString() << endl;

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
		QMetaObject::invokeMethod(this, "addContext",
				Qt::QueuedConnection,
				Q_ARG(const QString&, uri));
	} else {
		previousIp = "";
	}
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

bool ToolLauncher_API::connect(const QString& uri)
{
	QPushButton *btn = nullptr;

	for (auto it = tl->devices.begin();
			!btn && it != tl->devices.end(); ++it) {
		QPushButton *tmp = (*it)->second.btn;

		if (tmp->property("uri").toString().compare(uri) == 0)
			btn = tmp;
	}

	if (!btn)
		btn = tl->addContext(uri);

	btn->click();
	tl->ui->btnConnect->click();
}

void ToolLauncher_API::addIp(const QString& ip)
{
	if (!ip.isEmpty())
		QtConcurrent::run(std::bind(&ToolLauncher::checkIp, tl, ip));
}
