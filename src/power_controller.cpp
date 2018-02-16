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

#include "dynamicWidget.hpp"
#include "power_controller.hpp"
#include "filter.hpp"

#include "ui_powercontrol.h"

#include <iio.h>

#define TIMER_TIMEOUT_MS	200

using namespace adiscope;

PowerController::PowerController(struct iio_context *ctx,
		QPushButton *runButton, QJSEngine *engine,
		ToolLauncher *parent) :
	Tool(ctx, runButton, new PowerController_API(this), "Power Supply", parent),
	ui(new Ui::PowerController), in_sync(false)
{
	ui->setupUi(this);
	struct iio_device *dev1 = iio_context_find_device(ctx, "ad5627");
	struct iio_device *dev2 = iio_context_find_device(ctx, "ad9963");
	struct iio_device *dev3 = iio_context_find_device(ctx, "m2k-fabric");

	if (!dev1 || !dev2 || !dev3)
		throw std::runtime_error("Unable to find device\n");

	this->ch1w = iio_device_find_channel(dev1, "voltage0", true);
	this->ch2w = iio_device_find_channel(dev1, "voltage1", true);
	this->ch1r = iio_device_find_channel(dev2, "voltage2", false);
	this->ch2r = iio_device_find_channel(dev2, "voltage1", false);
	this->pd_pos = iio_device_find_channel(dev3, "voltage2", true);
	this->pd_neg = iio_device_find_channel(dev3, "voltage3", true); /* For HW Rev. >= C */

	if (!ch1w || !ch2w || !ch1r || !ch2r || !pd_pos)
		throw std::runtime_error("Unable to find channels\n");

	/* FIXME: TODO: Move this into a HW class / lib M2k
	 * This should be part of some pre-init call, where*/
	if (1) {
		struct iio_channel *chan;
		/* These are the two ADC amplifiers */
		chan = iio_device_find_channel(dev3, "voltage0", false);
		if (chan)
			iio_channel_attr_write_bool(chan, "powerdown", false);

		chan = iio_device_find_channel(dev3, "voltage1", false);
		if (chan)
			iio_channel_attr_write_bool(chan, "powerdown", false);

		/* ADF4360 globaal clock power down */
		iio_device_attr_write(dev3, "clk_powerdown", "0");
	}

	/* Power down DACs by default */
	iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", true);
	if (pd_neg)
		iio_channel_attr_write_bool(pd_neg, "user_supply_powerdown", true);
	iio_channel_attr_write_bool(ch1w, "powerdown", true);
	iio_channel_attr_write_bool(ch2w, "powerdown", true);

	/* Set the default values */
	iio_channel_attr_write_longlong(ch1w, "raw", 0LL);
	iio_channel_attr_write_longlong(ch2w, "raw", 0LL);

	connect(&this->timer, SIGNAL(timeout()), this, SLOT(update_lcd()));

	connect(ui->dac1, SIGNAL(toggled(bool)), this,
			SLOT(dac1_set_enabled(bool)));
	connect(ui->dac2, SIGNAL(toggled(bool)), this,
			SLOT(dac2_set_enabled(bool)));
	connect(ui->sync, SIGNAL(toggled(bool)), this,
			SLOT(sync_enabled(bool)));
	connect(ui->valuePos, SIGNAL(valueChanged(double)), this,
			SLOT(dac1_set_value(double)));
	connect(ui->valueNeg, SIGNAL(valueChanged(double)), this,
			SLOT(dac2_set_value(double)));
	connect(ui->trackingRatio, SIGNAL(valueChanged(int)), this,
			SLOT(ratioChanged(int)));

	connect(runButton, SIGNAL(clicked(bool)), this, SLOT(startStop(bool)));

	api->setObjectName(QString::fromStdString(Filter::tool_name(
			TOOL_POWER_CONTROLLER)));
	api->load(*settings);
	api->js_register(engine);
}

PowerController::~PowerController()
{
	/* Power down DACs */
	iio_channel_attr_write_bool(ch1w, "powerdown", true);
	iio_channel_attr_write_bool(ch2w, "powerdown", true);
	iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", true);
	if (pd_neg)
		iio_channel_attr_write_bool(pd_neg, "user_supply_powerdown", true);

	/* FIXME: TODO: Move this into a HW class / lib M2k */
	struct iio_device *dev3 = iio_context_find_device(ctx, "m2k-fabric");
	if (dev3) {
		struct iio_channel *chan;
		/* These are the two ADC amplifiers */
		chan = iio_device_find_channel(dev3, "voltage0", false);
		if (chan)
			iio_channel_attr_write_bool(chan, "powerdown", true);

		chan = iio_device_find_channel(dev3, "voltage1", false);
		if (chan)
			iio_channel_attr_write_bool(chan, "powerdown", true);

		/* ADF4360 globaal clock power down */
		iio_device_attr_write(dev3, "clk_powerdown", "1");
	}

	if (saveOnExit) {
		api->save(*settings);
	}
	delete api;

	delete ui;
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
	long long val = value * 4095.0 / (5.02 * 1.2);

	iio_channel_attr_write_longlong(ch1w, "raw", val);

	if (in_sync) {
		value = -value * ui->trackingRatio->value() / 100.0;
		ui->valueNeg->setValue(value);
		dac2_set_value(value);
	}
}

void PowerController::dac2_set_value(double value)
{
	long long val = value * 4095.0 / (-5.1 * 1.2);

	iio_channel_attr_write_longlong(ch2w, "raw", val);
}

void PowerController::dac1_set_enabled(bool enabled)
{
	iio_channel_attr_write_bool(ch1w, "powerdown", !enabled);

	if (in_sync)
		dac2_set_enabled(enabled);

	if (pd_neg) { /* For HW Rev. >= C */
		run_button->setChecked(enabled);
		iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", !enabled);
	} else {
		if (enabled) {
			run_button->setChecked(true);
			iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", false);
		} else if (!ui->dac2->isChecked()) {
			run_button->setChecked(false);
			iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", true);
		}
	}

	setDynamicProperty(ui->dac1, "running", enabled);
}

void PowerController::dac2_set_enabled(bool enabled)
{
	iio_channel_attr_write_bool(ch2w, "powerdown", !enabled);

	if (pd_neg) { /* For HW Rev. >= C */
		run_button->setChecked(enabled);
		iio_channel_attr_write_bool(pd_neg, "user_supply_powerdown", !enabled);
	} else {
		if (enabled) {
			run_button->setChecked(true);
			iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", false);
		} else if (!ui->dac1->isChecked()) {
			run_button->setChecked(false);
			iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", true);
		}
	}

	setDynamicProperty(ui->dac2, "running", enabled);
}

void PowerController::sync_enabled(bool enabled)
{
	if (ui->dac1->isChecked()) {
		dac2_set_enabled(enabled);
		ui->dac2->setChecked(enabled);
	}

	in_sync = enabled;
	ui->valueNeg->setDisabled(enabled);
	ui->valueNeg->setValue(-ui->valuePos->value() *
			(double) ui->trackingRatio->value() / 100.0);
}

void PowerController::ratioChanged(int percent)
{
	ui->valueNeg->setValue(-ui->valuePos->value() *
			(double) percent / 100.0);
}

void PowerController::update_lcd()
{
	long long val1 = 0, val2 = 0;

	iio_channel_attr_read_longlong(ch1r, "raw", &val1);
	iio_channel_attr_read_longlong(ch2r, "raw", &val2);

	double value1 = (double) val1 * 6.4 / 4095.0;
	ui->lcd1->display(value1);
	ui->scale_dac1->setValue(value1);

	double value2 = (double) val2 * -6.4 / 4095.0;
	ui->lcd2->display(value2);
	ui->scale_dac2->setValue(value2);

	timer.start(TIMER_TIMEOUT_MS);
}

void PowerController::startStop(bool start)
{
	dac1_set_enabled(start);
	ui->dac1->setChecked(start);

	dac2_set_enabled(start);
	ui->dac2->setChecked(start);
}

bool PowerController_API::syncEnabled() const
{
	return pw->ui->sync->isChecked();
}

void PowerController_API::enableSync(bool en)
{
	if (en)
		pw->ui->sync->click();
	else
		pw->ui->notSync->click();
}

int PowerController_API::getTrackingPercent() const
{
	return pw->ui->trackingRatio->value();
}

void PowerController_API::setTrackingPercent(int percent)
{
	pw->ui->trackingRatio->setValue(percent);
}

double PowerController_API::valueDac1() const
{
	return pw->ui->valuePos->value();
}

void PowerController_API::setValueDac1(double value)
{
	pw->ui->valuePos->setValue(value);
}

double PowerController_API::valueDac2() const
{
	return pw->ui->valueNeg->value();
}

void PowerController_API::setValueDac2(double value)
{
	if (!syncEnabled())
		pw->ui->valueNeg->setValue(value);
}
