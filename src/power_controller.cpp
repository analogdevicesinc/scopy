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

#define TIMER_TIMEOUT_MS	200

using namespace adiscope;

PowerController::PowerController(struct iio_context *ctx,
		ToolMenuItem *toolMenuItem, QJSEngine *engine,
		ToolLauncher *parent) :
	Tool(ctx, toolMenuItem, new PowerController_API(this), "Power Supply", parent),
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

	ui->btnSync->click();

	valuePos = new PositionSpinButton({
		{"mVolts",1e-3},
		{"Volts",1e0}
	}, "Value", 0, 5, true, true, this);

	valueNeg = new PositionSpinButton({
		{"mVolts",1e-3},
		{"Volts",1e0}
	}, "Value", -5, 0, true, true, this);

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

	/*Load calibration parameters from iio context*/
	const char *name;
	const char *value;
	for (int i = 4; i < 12; i++) {
		if (!iio_context_get_attr(ctx, i, &name, &value))
			calibrationParam[QString(name + 4)] = QString(value).toDouble();
	}

	api->setObjectName(QString::fromStdString(Filter::tool_name(
							  TOOL_POWER_CONTROLLER)));
	api->load(*settings);
	api->js_register(engine);

}

PowerController::~PowerController()
{
	ui->dac1->setChecked(false);
	ui->dac2->setChecked(false);

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
	double offset = calibrationParam[QString("offset_pos_dac")];
	double gain = calibrationParam[QString("gain_pos_dac")];

	long long val = (value * gain + offset)  * 4095.0 / (5.02 * 1.2 ) ;

	if (val < 0 )
		val = 0;

	iio_channel_attr_write_longlong(ch1w, "raw", val);
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
	double offset = calibrationParam[QString("offset_neg_dac")];
	double gain = calibrationParam[QString("gain_neg_dac")];

	long long val = (value * gain + offset) * 4095.0 / (-5.1 * 1.2 );

	if (val < 0 )
		val = 0;

	iio_channel_attr_write_longlong(ch2w, "raw", val);
	averageVoltageCh2.clear();
}

void PowerController::dac1_set_enabled(bool enabled)
{
	iio_channel_attr_write_bool(ch1w, "powerdown", !enabled);
	averageVoltageCh1.clear();

	if (in_sync)
		dac2_set_enabled(enabled);

	if (pd_neg) { /* For HW Rev. >= C */
		iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", !enabled);
	} else {
		if (enabled) {
			iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", false);
		} else if (!ui->dac2->isChecked()) {
			iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", true);
		}
	}

	setDynamicProperty(ui->dac1, "running", enabled);
}

void PowerController::dac2_set_enabled(bool enabled)
{
	iio_channel_attr_write_bool(ch2w, "powerdown", !enabled);
	averageVoltageCh2.clear();

	if (pd_neg) { /* For HW Rev. >= C */
		iio_channel_attr_write_bool(pd_neg, "user_supply_powerdown", !enabled);
	} else {
		if (enabled) {
			iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", false);
		} else if (!ui->dac1->isChecked()) {
			iio_channel_attr_write_bool(pd_pos, "user_supply_powerdown", true);
		}
	}

	setDynamicProperty(ui->dac2, "running", enabled);
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
	double offset1 = calibrationParam[QString("offset_pos_adc")];
	double gain1 = calibrationParam[QString("gain_pos_adc")];
	double offset2 = calibrationParam[QString("offset_neg_adc")];
	double gain2 = calibrationParam[QString("gain_neg_adc")];

	long long val1 = 0, val2 = 0;
	double average1 = 0, average2 = 0;

	iio_channel_attr_read_longlong(ch1r, "raw", &val1);
	iio_channel_attr_read_longlong(ch2r, "raw", &val2);

	averageVoltageCh1.push_back(val1);
	averageVoltageCh2.push_back(val2);

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

	double value1 = (((double) average1 * 6.4 / 4095.0) + offset1) * gain1;
	ui->lcd1->display(value1);
	ui->scale_dac1->setValue(value1);

	double value2 = (((double) average2 * (-6.4)  / 4095.0) + offset2) * gain2;
	ui->lcd2->display(value2);
	ui->scale_dac2->setValue(value2);

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

