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

#include "adc_sample_conv.hpp"
#include "dmm.hpp"
#include "ui_dmm.h"

#include <gnuradio/blocks/moving_average_ff.h>
#include <gnuradio/blocks/rms_ff.h>
#include <gnuradio/blocks/short_to_float.h>
#include <gnuradio/blocks/sub_ff.h>

#include <QJSEngine>

using namespace adiscope;

DMM::DMM(struct iio_context *ctx, Filter *filt, QPushButton *runButton,
		QJSEngine *engine,
		float gain_ch1, float gain_ch2, QWidget *parent) :
	QWidget(parent), ui(new Ui::DMM), timer(this),
	manager(iio_manager::get_instance(ctx, filt->device_name(TOOL_DMM))),
	peek_block_ch1(gnuradio::get_initial_sptr(new peek_sample<float>)),
	peek_block_ch2(gnuradio::get_initial_sptr(new peek_sample<float>)),
	mode_ac_ch1(false), mode_ac_ch2(false),
	gain_ch1(gain_ch1), gain_ch2(gain_ch2), dmm_api(new DMM_API(this))
{
	ui->setupUi(this);

	ui->sismograph_ch1->setColor(QColor("#ff7200"));
	ui->sismograph_ch2->setColor(QColor("#9013fe"));

	connect(&timer, SIGNAL(timeout()), this, SLOT(updateValuesList()));

	connect(ui->run_button, SIGNAL(toggled(bool)),
			this, SLOT(toggleTimer(bool)));
	connect(runButton, SIGNAL(toggled(bool)), this,
			SLOT(toggleTimer(bool)));
	connect(ui->run_button, SIGNAL(toggled(bool)),
			runButton, SLOT(setChecked(bool)));
	connect(runButton, SIGNAL(toggled(bool)), ui->run_button,
			SLOT(setChecked(bool)));

	connect(ui->btn_ch1_ac, SIGNAL(toggled(bool)),
			this, SLOT(toggleAC1(bool)));
	connect(ui->btn_ch2_ac, SIGNAL(toggled(bool)),
			this, SLOT(toggleAC2(bool)));

	connect(ui->historySizeCh1, SIGNAL(currentIndexChanged(int)),
			this, SLOT(setHistorySizeCh1(int)));
	connect(ui->historySizeCh2, SIGNAL(currentIndexChanged(int)),
			this, SLOT(setHistorySizeCh2(int)));

	setHistorySizeCh1(ui->historySizeCh1->currentIndex());
	setHistorySizeCh2(ui->historySizeCh2->currentIndex());

	/* Lock the flowgraph if we are already started */
	bool started = manager->started();
	if (started)
		manager->lock();

	id_ch1 = configureMode(ui->btn_ch1_ac->isChecked(), 0);
	id_ch2 = configureMode(ui->btn_ch2_ac->isChecked(), 1);

	if (started)
		manager->unlock();

	dmm_api->load();
	dmm_api->js_register(engine);
}

void DMM::disconnectAll()
{
	bool started = manager->started();
	if (started)
		manager->lock();

	manager->disconnect(id_ch1);
	manager->disconnect(id_ch2);

	if (started)
		manager->unlock();
}

DMM::~DMM()
{
	timer.stop();
	disconnectAll();

	dmm_api->save();
	delete dmm_api;

	delete ui;
}

void DMM::updateValuesList()
{
	float value_ch1 = peek_block_ch1->peek_value();
	float value_ch2 = peek_block_ch2->peek_value();

	/* XXX: This is M2K specific! */
	float volts_ch1 = adc_sample_conv::convSampleToVolts(
			(float) value_ch1, gain_ch1);
	float volts_ch2 = adc_sample_conv::convSampleToVolts(
			(float) value_ch2, gain_ch2);

	ui->lcdCh1->display(volts_ch1);
	ui->lcdCh2->display(volts_ch2);

	ui->scaleCh1->setValue(volts_ch1);
	ui->scaleCh2->setValue(volts_ch2);

	ui->sismograph_ch1->plot(volts_ch1);
	ui->sismograph_ch2->plot(volts_ch2);
}

void DMM::toggleTimer(bool start)
{
	if (start) {
		manager->start(id_ch1);
		manager->start(id_ch2);

		timer.start(100);
		ui->run_button->setText("Stop");

		ui->scaleCh1->start();
		ui->scaleCh2->start();
	} else {
		ui->scaleCh1->stop();
		ui->scaleCh2->stop();

		ui->run_button->setText("Run");
		timer.stop();

		manager->stop(id_ch1);
		manager->stop(id_ch2);
	}

	ui->run_button->setChecked(start);
}

iio_manager::port_id DMM::configureMode(bool is_ac, unsigned int ch)
{
	auto s2f = gr::blocks::short_to_float::make();

	/* XXX: hardcoded parameters! */
	auto avg = gr::blocks::moving_average_ff::make(1e6, 1e-6);

	iio_manager::port_id id = manager->connect(s2f, ch, 0);

	manager->connect(s2f, 0, avg, 0);

	if (is_ac) {
		auto sub = gr::blocks::sub_ff::make();
		auto rms = gr::blocks::rms_ff::make(0.001);

		manager->connect(s2f, 0, sub, 0);
		manager->connect(avg, 0, sub, 1);
		manager->connect(sub, 0, rms, 0);

		/* The 'peek block' is basically a null sink, reading its input one
		 * sample at a time. It provides a peek_value() function to retrieve
		 * the last sample read. */

		if (ch == 0) {
			manager->connect(rms, 0, peek_block_ch1, 0);
		} else {
			manager->connect(rms, 0, peek_block_ch2, 0);
		}

	} else {
		if (ch == 0)
			manager->connect(avg, 0, peek_block_ch1, 0);
		else
			manager->connect(avg, 0, peek_block_ch2, 0);
	}

	return id;
}

void DMM::toggleAC1(bool enable)
{
	bool started = manager->started();
	if (started)
		manager->lock();

	manager->disconnect(id_ch1);

	id_ch1 = configureMode(enable, 0);
	mode_ac_ch1 = enable;

	if (started) {
		manager->start(id_ch1);
		manager->unlock();
	}

	ui->labelCh1->setText(enable ? "VRMS" : "VDC");
	if (enable)
		ui->btn_ch1_ac->setChecked(true);
	else
		ui->btn_ch1_dc->setChecked(true);
}

void DMM::toggleAC2(bool enable)
{
	bool started = manager->started();
	if (started)
		manager->lock();

	manager->disconnect(id_ch2);

	id_ch2 = configureMode(enable, 1);
	mode_ac_ch2 = enable;

	if (started) {
		manager->start(id_ch2);
		manager->unlock();
	}

	ui->labelCh2->setText(enable ? "VRMS" : "VDC");
	if (enable)
		ui->btn_ch2_ac->setChecked(true);
	else
		ui->btn_ch2_dc->setChecked(true);
}

int DMM::numSamplesFromIdx(int idx)
{
	switch(idx) {
	case 0:
		return 10;
	case 1:
		return 100;
	case 2:
		return 600;
	default:
		throw std::runtime_error("Invalid IDX");
	}
}

void DMM::setHistorySizeCh1(int idx)
{
	int num_samples = numSamplesFromIdx(idx);

	ui->sismograph_ch1->setNumSamples(num_samples);
}

void DMM::setHistorySizeCh2(int idx)
{
	int num_samples = numSamplesFromIdx(idx);

	ui->sismograph_ch2->setNumSamples(num_samples);
}
