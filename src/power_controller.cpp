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

#include "dynamicWidget.hpp"
#include "power_controller.hpp"
#include "filter.hpp"

#include "ui_powercontrol.h"

#include <iio.h>
#include "power_controller_api.hpp"
#include "logging_categories.h"

/* libm2k includes */
#include <libm2k/contextbuilder.hpp>
#include <libm2k/m2k.hpp>
#include <libm2k/m2kexceptions.hpp>
#include <libm2k/analog/m2kpowersupply.hpp>
#include "scopyExceptionHandler.h"

#define TIMER_TIMEOUT_MS	200

using namespace adiscope;
using namespace libm2k::context;
using namespace libm2k::analog;

PowerController::PowerController(struct iio_context *ctx,
		ToolMenuItem *toolMenuItem, QJSEngine *engine,
		ToolLauncher *parent) :
    Tool(ctx, toolMenuItem, new PowerController_API(this), "Power Supply", parent),
	ui(new Ui::PowerController), in_sync(false),
	m_m2k_context(m2kOpen(ctx, "")),
	m_m2k_powersupply(m_m2k_context->getPowerSupply())
{
	ui->setupUi(this);

	try {
		m_m2k_powersupply->enableChannel(0, false);
		m_m2k_powersupply->enableChannel(1, false);
		m_m2k_powersupply->pushChannel(0, 0.0);
		m_m2k_powersupply->pushChannel(1, 0.0);
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e);
		qDebug(CAT_POWER_CONTROLLER) << "Can't write push value: " << e.what();
	}

	ui->btnSync->click();

	valuePos = new PositionSpinButton({
	{tr("mVolts"),1e-3},
	{tr("Volts"),1e0}
	}, tr("Value"), 0, 5, true, true, this);

	valueNeg = new PositionSpinButton({
	{tr("mVolts"),1e-3},
	{tr("Volts"),1e0}
	}, tr("Value"), -5, 0, true, true, this);

	ui->valuePosLayout->addWidget(valuePos);
	ui->valueNegLayout->addWidget(valueNeg);

	connect(valuePos, &PositionSpinButton::valueChanged,
		ui->lcd1_set, &LcdNumber::display);
	connect(valueNeg, &PositionSpinButton::valueChanged,
		ui->lcd2_set, &LcdNumber::display);

	connect(&this->timer, SIGNAL(timeout()), this, SLOT(update_lcd()));

	connect(ui->dac1, SIGNAL(toggled(bool)), this,
			SLOT(dac1_set_enabled(bool)));
	connect(ui->dac2, SIGNAL(toggled(bool)), this,
			SLOT(dac2_set_enabled(bool)));
	connect(ui->btnSync, SIGNAL(toggled(bool)), this,
			SLOT(sync_enabled(bool)));
	connect(valuePos, SIGNAL(valueChanged(double)), this,
			SLOT(dac1_set_value(double)));
	connect(valueNeg, SIGNAL(valueChanged(double)), this,
			SLOT(dac2_set_value(double)));
	connect(ui->trackingRatio, SIGNAL(valueChanged(int)), this,
			SLOT(ratioChanged(int)));

	connect(runButton(), SIGNAL(clicked(bool)), this, SLOT(startStop(bool)));

	valuePos->setValue(0);
	valueNeg->setValue(0);

	connect(ui->dac1, &QPushButton::toggled, this, &PowerController::toggleRunButton);
	connect(ui->dac2, &QPushButton::toggled, this, &PowerController::toggleRunButton);

	api->setObjectName(QString::fromStdString(Filter::tool_name(
							  TOOL_POWER_CONTROLLER)));
	api->load(*settings);
	api->js_register(engine);

	readPreferences();

}

PowerController::~PowerController()
{
	disconnect(prefPanel, &Preferences::notify, this, &PowerController::readPreferences);
	ui->dac1->setChecked(false);
	ui->dac2->setChecked(false);

	try {
		m_m2k_powersupply->powerDownDacs(true);
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e);
		qDebug(CAT_POWER_CONTROLLER) << e.what();
	}

	if (saveOnExit) {
		api->save(*settings);
	}
	delete api;

	delete ui;
}

void PowerController::readPreferences()
{
	ui->instrumentNotes->setVisible(prefPanel->getInstrumentNotesActive());
}

void PowerController::toggleRunButton(bool enabled)
{
	bool dac1Enabled = ui->dac1->isChecked();
	bool dac2Enabled = ui->dac2->isChecked();
	if (enabled) {
		run_button->setChecked(enabled);
	} else {
		run_button->setChecked(dac1Enabled | dac2Enabled);
	}
}

void PowerController::showEvent(QShowEvent *event)
{
	timer.start(TIMER_TIMEOUT_MS);
}

void PowerController::hideEvent(QHideEvent *event)
{
	timer.stop();
}

void PowerController::dac1_set_value(double value)
{
	try {
		m_m2k_powersupply->pushChannel(0, value);
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e);
		qDebug(CAT_POWER_CONTROLLER) << "Can't write push value: " << e.what();
	}
	averageVoltageCh1.clear();

	if (in_sync) {
		value = -value * ui->trackingRatio->value() / 100.0;
		valueNeg->setValue(value);
		dac2_set_value(value);
		averageVoltageCh2.clear();
	}
}

void PowerController::dac2_set_value(double value)
{
	try {
		m_m2k_powersupply->pushChannel(1, value);
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e);
		qDebug(CAT_POWER_CONTROLLER) << "Can't write push value: " << e.what();
	}
	averageVoltageCh2.clear();
}

void PowerController::dac1_set_enabled(bool enabled)
{
	try {
		m_m2k_powersupply->enableChannel(0, enabled);
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e);
		qDebug(CAT_POWER_CONTROLLER) << "Can't enable channel: " << e.what();
	}
	averageVoltageCh1.clear();

	if (in_sync)
		dac2_set_enabled(enabled);

	setDynamicProperty(ui->dac1, "running", enabled);
	ui->dac1->setText(enabled ? tr("Disable") : tr("Enable"));
}

void PowerController::dac2_set_enabled(bool enabled)
{
	try {
		m_m2k_powersupply->enableChannel(1, enabled);
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e);
		qDebug(CAT_POWER_CONTROLLER) << "Can't enable channel: " << e.what();
	}
	averageVoltageCh2.clear();
	setDynamicProperty(ui->dac2, "running", enabled);
	ui->dac2->setText(enabled ? tr("Disable") : tr("Enable"));
}

void PowerController::sync_enabled(bool enabled)
{
	if (ui->dac1->isChecked()) {
		dac2_set_enabled(!enabled);
		ui->dac2->setChecked(!enabled);
	}

	in_sync = !enabled;
	valueNeg->setDisabled(!enabled);
	valueNeg->setValue(-valuePos->value() *
			(double) ui->trackingRatio->value() / 100.0);
}

void PowerController::ratioChanged(int percent)
{
	valueNeg->setValue(-valuePos->value() *
			(double) percent / 100.0);
}

void PowerController::update_lcd()
{
	double average1 = 0, average2 = 0;
	double value1 = 0.0, value2 = 0.0;

	try {
		value1 = m_m2k_powersupply->readChannel(0);
		value2 = m_m2k_powersupply->readChannel(1);
	} catch (libm2k::m2k_exception &e) {
		HANDLE_EXCEPTION(e);
		qDebug(CAT_POWER_CONTROLLER) << "Can't read value: " << e.what();
	}

	averageVoltageCh1.push_back(value1);
	averageVoltageCh2.push_back(value2);

	if(averageVoltageCh1.length() > AVERAGE_COUNT)
		averageVoltageCh1.pop_front();
	if(averageVoltageCh2.length() > AVERAGE_COUNT)
		averageVoltageCh2.pop_front();

	for (int i = 0; i < averageVoltageCh1.size(); ++i)
		average1 += averageVoltageCh1.at(i);
	for (int i = 0; i < averageVoltageCh2.size(); ++i)
		average2 += averageVoltageCh2.at(i);

	average1  /= averageVoltageCh1.length();
	average2  /= averageVoltageCh2.length();

	ui->lcd1->display(average1);
	ui->scale_dac1->setValue(average1);

	ui->lcd2->display(average2);
	ui->scale_dac2->setValue(average2);

	timer.start(TIMER_TIMEOUT_MS);
}

void PowerController::run()
{
	startStop(true);
}
void PowerController::stop()
{
	startStop(false);
}

void PowerController::startStop(bool start)
{
	dac1_set_enabled(start);
	ui->dac1->setChecked(start);

	dac2_set_enabled(start);
	ui->dac2->setChecked(start);
	m_running = start;
}

