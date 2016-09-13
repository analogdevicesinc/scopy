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

#include "trigger_settings.hpp"
#include "adc_sample_conv.hpp"
#include "spinbox_a.hpp"

#include "ui_trigger_settings.h"

#include <iio.h>

using namespace adiscope;
using namespace std;

vector<string> TriggerSettings::lut_trigger_sources = {
	"a",
	"b",
	"a_OR_b",
	"a_AND_b",
	"a_XOR_b",
};

vector<string> TriggerSettings::lut_triggerX_types = {
	"always",
	"analog",
	"digital",
	"digital_OR_analog",
	"digital_AND_analog",
	"digital_XOR_analog",
	"!digital_OR_analog",
	"!digital_AND_analog",
	"!digital_XOR_analog",
};

vector<string> TriggerSettings::lut_analog_trigger_conditions = {
	"edge-rising",
	"edge-falling",
	"level-low",
	"level-high",
};

vector<string> TriggerSettings::lut_digital_trigger_conditions = {
	"edge-rising",
	"edge-falling",
	"edge-any",
	"level-low",
	"level-high",
};

TriggerSettings::TriggerSettings(struct iio_context *ctx, QWidget *parent) :
	QWidget(parent), ui(new Ui::TriggerSettings),
	triggerA_en(false), triggerB_en(false),
	plot_num_samples(0)
{
	ui->setupUi(this);

	ui_triggerDelay = new PositionSpinButton({
				{"ns", 1E-9},
				{"μs", 1E-6},
				{"ms", 1E-3},
				{"s", 1E0}
				}, "Position",
				-1E3,
				1E3);
	ui_triggerDelay->setStep(1E-6);

	ui_triggerAlevel = new PositionSpinButton({
						{"μVolts", 1E-6},
						{"mVolts", 1E-3},
						{"Volts", 1E0}
						},
						"Level",
						-25, +25);
	ui_triggerBlevel = new PositionSpinButton({
						{"μVolts", 1E-6},
						{"mVolts", 1E-3},
						{"Volts", 1E0}
						},
						"Level",
						-25, +25);
	ui_triggerAHyst = new PositionSpinButton({
						{"μVolts", 1E-6},
						{"mVolts", 1E-3},
						{"Volts", 1E0}
						},
						"Hyst A",
						-25, +25,
						false);
	ui_triggerBHyst = new PositionSpinButton({
						{"μVolts", 1E-6},
						{"mVolts", 1E-3},
						{"Volts", 1E0}
						},
						"Hyst B",
						-25, +25,
						false);
	ui_triggerHoldoff = new PositionSpinButton({
						{"μs", 1E-6},
						{"ms", 1E-3},
						{"s", 1E0}
						},
						"Holdoff",
						0, 1,
						false);

	ui->triggerDelay_container->addWidget(ui_triggerDelay, 0, Qt::AlignLeft);
	ui->triggerAlevel_container->addWidget(ui_triggerAlevel, 0, Qt::AlignLeft);
	ui->triggerBlevel_container->addWidget(ui_triggerBlevel, 0, Qt::AlignLeft);
	ui->hysterezisA_container->addWidget(ui_triggerAHyst);
	ui->hysterezisB_container->addWidget(ui_triggerBHyst);
	ui->holdoff_container->addWidget(ui_triggerHoldoff);

	if (ctx) {
		struct iio_device *dev = iio_context_find_device(ctx,
							"m2k-adc-trigger");
		if (dev) {
			this->trigger0 = iio_device_find_channel(dev,
							"voltage0", false);
			this->trigger1 = iio_device_find_channel(dev,
							"voltage1", false);
			this->digitalTrigger0 = iio_device_find_channel(dev,
							"voltage2", false);
			this->digitalTrigger1 = iio_device_find_channel(dev,
							"voltage3", false);
			this->trigger0Mode = iio_device_find_channel(dev,
							"voltage4", false);
			this->trigger1Mode = iio_device_find_channel(dev,
							"voltage5", false);
			this->timeTrigger = iio_device_find_channel(dev,
							"voltage6", false);

			trigger_all_widgets_update();

			on_btn_noise_reject_toggled(false);
			onSpinboxTriggerAhystChanged(0);
			onSpinboxTriggerBhystChanged(0);
		}
	}

	connect(ui_triggerDelay, SIGNAL(valueChanged(double)),
		SLOT(onSpinboxTriggerDelayChanged(double)));
	connect(ui_triggerAlevel, SIGNAL(valueChanged(double)),
		SLOT(onSpinboxTriggerAlevelChanged(double)));
	connect(ui_triggerBlevel, SIGNAL(valueChanged(double)),
		SLOT(onSpinboxTriggerBlevelChanged(double)));
	connect(ui_triggerAHyst, SIGNAL(valueChanged(double)),
		SLOT(onSpinboxTriggerAhystChanged(double)));
	connect(ui_triggerBHyst, SIGNAL(valueChanged(double)),
		SLOT(onSpinboxTriggerBhystChanged(double)));
}

TriggerSettings::~TriggerSettings()
{
	delete ui;
}

double TriggerSettings::levelA_value()
{
	return ui_triggerAlevel->value();
}

double TriggerSettings::levelB_value()
{
	return ui_triggerBlevel->value();
}

bool TriggerSettings::levelA_enabled()
{
	return triggerA_en;
}

bool TriggerSettings::levelB_enabled()
{
	return triggerB_en;
}

void TriggerSettings::setDelay(double seconds)
{
	double current_delay = ui_triggerDelay->value();

	if (current_delay != seconds) {
		ui_triggerDelay->setValue(seconds);
		trigg_delay_write_hardware(seconds);
	}
}

void TriggerSettings::setTriggerLevelA(double level)
{
	double current_level = ui_triggerAlevel->value();

	if (current_level != level)
		ui_triggerAlevel->setValue(level);
}

void TriggerSettings::setTriggerLevelB(double level)
{
	double current_level = ui_triggerBlevel->value();

	if (current_level != level)
		ui_triggerBlevel->setValue(level);
}

void TriggerSettings::on_cmb_trigg_source_currentIndexChanged(int index)
{
	bool triggerA_new_state;
	bool triggerB_new_state;

	if (this->timeTrigger) {
		iio_channel_attr_write(this->timeTrigger, "logic_mode",
				       lut_trigger_sources[index].c_str());
	}

	trigger_ab_enabled_update(triggerA_new_state, triggerB_new_state);
	if (triggerA_new_state != triggerA_en) {
		triggerA_en = triggerA_new_state;
		emit triggerAenabled(triggerA_new_state);
	}
	if (triggerB_new_state != triggerB_en) {
		triggerB_en = triggerB_new_state;
		emit triggerBenabled(triggerB_new_state);
	}
}

void TriggerSettings::on_cmb_trigg_A_currentIndexChanged(int index)
{
	if (this->trigger0Mode) {
		int i = index + 1;
		iio_channel_attr_write(this->trigger0Mode, "mode",
				       lut_triggerX_types[i].c_str());
	}

	ui_reconf_on_triggerA_mode_changed(index);
}

void TriggerSettings::on_cmb_trigg_B_currentIndexChanged(int index)
{
	if (this->trigger1Mode) {
		int i = index + 1;
		iio_channel_attr_write(this->trigger1Mode, "mode",
				       lut_triggerX_types[i].c_str());
	}

	ui_reconf_on_triggerB_mode_changed(index);
}

void TriggerSettings::onSpinboxTriggerDelayChanged(double value)
{
	emit delayChanged(value);
	trigg_delay_write_hardware(value);
}

void TriggerSettings::onSpinboxTriggerAlevelChanged(double value)
{
	emit levelAChanged(value);
	triggA_level_write_hardware(value);
}

void TriggerSettings::onSpinboxTriggerBlevelChanged(double value)
{
	emit levelBChanged(value);
	triggB_level_write_hardware(value);
}

void TriggerSettings::onSpinboxTriggerAhystChanged(double value)
{
	if (!this->trigger0)
		return;

	int rawValue = adc_sample_conv::convVoltsToSample(value);
	QString s = QString::number(rawValue);

	iio_channel_attr_write(this->trigger0, "trigger_hysteresis",
				s.toLocal8Bit().QByteArray::constData());
}

void TriggerSettings::onSpinboxTriggerBhystChanged(double value)
{
	if (!this->trigger1)
		return;

	int rawValue = adc_sample_conv::convVoltsToSample(value);
	QString s = QString::number(rawValue);

	iio_channel_attr_write(this->trigger1, "trigger_hysteresis",
				s.toLocal8Bit().QByteArray::constData());
}

void TriggerSettings::on_cmb_triggA_cond_currentIndexChanged(int index)
{
	if (!this->trigger0)
		return;

	if (index < ui->cmb_triggA_cond->count() - 1) {
		iio_channel_attr_write(this->trigger0, "trigger",
				lut_analog_trigger_conditions[index].c_str());
		int pos = ui->cmb_trigg_A->currentIndex();
		iio_channel_attr_write(this->trigger0Mode, "mode",
				       lut_triggerX_types[pos + 1].c_str());
	} else {
		iio_channel_attr_write(this->trigger0Mode, "mode",
				       lut_triggerX_types[0].c_str());
	}

	ui_reconf_on_triggerA_cond_changed(index);
}

void TriggerSettings::on_cmb_triggB_cond_currentIndexChanged(int index)
{
	if (!this->trigger1)
		return;

	if (index < ui->cmb_triggB_cond->count() - 1) {
		iio_channel_attr_write(this->trigger1, "trigger",
				lut_analog_trigger_conditions[index].c_str());
		int pos = ui->cmb_trigg_B->currentIndex();
		iio_channel_attr_write(this->trigger1Mode, "mode",
				       lut_triggerX_types[pos + 1].c_str());
	} else {
		iio_channel_attr_write(this->trigger1Mode, "mode",
				       lut_triggerX_types[0].c_str());
	}

	ui_reconf_on_triggerB_cond_changed(index);
}

void TriggerSettings::on_cmb_triggA_ext_cond_currentIndexChanged(int index)
{
	if (!this->digitalTrigger0)
		return;

	iio_channel_attr_write(this->digitalTrigger0, "trigger",
				lut_digital_trigger_conditions[index].c_str());
}

void TriggerSettings::on_cmb_triggB_ext_cond_currentIndexChanged(int index)
{
	if (!this->digitalTrigger1)
		return;

	iio_channel_attr_write(this->digitalTrigger1, "trigger",
				lut_digital_trigger_conditions[index].c_str());
}

void adiscope::TriggerSettings::on_trigg_A_extern_en_toggled(bool checked)
{
	ui_reconf_on_triggerA_extern_changed(checked);
}

void adiscope::TriggerSettings::on_trigg_B_extern_en_toggled(bool checked)
{
	ui_reconf_on_triggerB_extern_changed(checked);
}

void TriggerSettings::trigger_all_widgets_update()
{
	ssize_t ret;
	char buf[4096];

	if (this->trigger0) {
		ret = iio_channel_attr_read(this->trigger0, "trigger", buf, sizeof(buf));
		if (ret >= 0) {
			int pos = std::find(lut_analog_trigger_conditions.begin(),lut_analog_trigger_conditions.end(), buf) -
					lut_analog_trigger_conditions.begin();
			if (pos < lut_analog_trigger_conditions.size()) {
				ui->cmb_triggA_cond->setCurrentIndex(pos);
				ui_reconf_on_triggerA_cond_changed(pos);
			}
		}

		ret = iio_channel_attr_read(this->trigger0, "trigger_level", buf, sizeof(buf));
		if (ret >= 0) {
			double val = adc_sample_conv::convSampleToVolts(QString(buf).toDouble());
			ui_triggerAlevel->setValue(val);
		}

		ret = iio_channel_attr_read(this->trigger0, "trigger_hysteresis", buf, sizeof(buf));
		if (ret >= 0) {
			double val = adc_sample_conv::convSampleToVolts(QString(buf).toDouble());
			ui_triggerAHyst->setValue(val);
		}
	}

	if (this->trigger1) {
		ret = iio_channel_attr_read(this->trigger1, "trigger", buf, sizeof(buf));
		if (ret >= 0) {
			int pos = std::find(lut_analog_trigger_conditions.begin(),lut_analog_trigger_conditions.end(), buf) -
					lut_analog_trigger_conditions.begin();
			if (pos < lut_analog_trigger_conditions.size()) {
				ui->cmb_triggB_cond->setCurrentIndex(pos);
				ui_reconf_on_triggerB_cond_changed(pos);
			}
		}

		ret = iio_channel_attr_read(this->trigger1, "trigger_level", buf, sizeof(buf));
		if (ret >= 0) {
			double val = adc_sample_conv::convSampleToVolts(QString(buf).toDouble());
			ui_triggerBlevel->setValue(val);
		}

		ret = iio_channel_attr_read(this->trigger1, "trigger_hysteresis", buf, sizeof(buf));
		if (ret >= 0) {
			double val = adc_sample_conv::convSampleToVolts(QString(buf).toDouble());
			ui_triggerBHyst->setValue(val);
		}
	}


	if (this->digitalTrigger0) {
		ret = iio_channel_attr_read(this->digitalTrigger0, "trigger", buf, sizeof(buf));
		if (ret >= 0) {
			int pos = std::find(lut_digital_trigger_conditions.begin(), lut_digital_trigger_conditions.end(), buf) -
					lut_digital_trigger_conditions.begin();
			if (pos < lut_digital_trigger_conditions.size())
				ui->cmb_triggA_ext_cond->setCurrentIndex(pos);
		}
	}

	if (this->digitalTrigger1) {
		ret = iio_channel_attr_read(this->digitalTrigger1, "trigger", buf, sizeof(buf));
		if (ret >= 0) {
			int pos = std::find(lut_digital_trigger_conditions.begin(), lut_digital_trigger_conditions.end(), buf) -
					lut_digital_trigger_conditions.begin();
			if (pos < lut_digital_trigger_conditions.size())
				ui->cmb_triggB_ext_cond->setCurrentIndex(pos);
		}
	}

	if (this->trigger0Mode) {
		ret = iio_channel_attr_read(this->trigger0Mode, "mode", buf, sizeof(buf));
		if (ret >= 0) {
			int pos = std::find(lut_triggerX_types.begin(), lut_triggerX_types.end(), buf) -
					lut_triggerX_types.begin();
			if (pos < lut_triggerX_types.size()) {
				if (pos != 0) {
					ui->cmb_trigg_A->setCurrentIndex(pos - 1);
					ui_reconf_on_triggerA_mode_changed(pos - 1);
					if (ui->cmb_trigg_A->currentIndex() == 0)
						ui_reconf_on_triggerA_extern_changed(false);
				} else {
					int last = ui->cmb_triggA_cond->count() - 1;
					ui->cmb_triggA_cond->setCurrentIndex(last);
					ui_reconf_on_triggerA_cond_changed(last);
					ui_reconf_on_triggerA_extern_changed(false);
				}
			}
		}
	}

	if (this->trigger1Mode) {
		ret = iio_channel_attr_read(this->trigger1Mode, "mode", buf, sizeof(buf));
		if (ret >= 0) {
			int pos = std::find(lut_triggerX_types.begin(), lut_triggerX_types.end(), buf) -
					lut_triggerX_types.begin();
			if (pos < lut_triggerX_types.size()) {
				if (pos != 0) {
					ui->cmb_trigg_B->setCurrentIndex(pos - 1);
					ui_reconf_on_triggerB_mode_changed(pos - 1);
					if (ui->cmb_trigg_B->currentIndex() == 0)
						ui_reconf_on_triggerB_extern_changed(false);
				} else {
					int last = ui->cmb_triggB_cond->count() - 1;
					ui->cmb_triggB_cond->setCurrentIndex(last);
					ui_reconf_on_triggerB_cond_changed(last);
					ui_reconf_on_triggerB_extern_changed(false);
				}
			}
		}
	}

	if (this->timeTrigger) {
		ret = iio_channel_attr_read(this->timeTrigger, "logic_mode", buf, sizeof(buf));
		if (ret >= 0) {
			int pos = std::find(lut_trigger_sources.begin(), lut_trigger_sources.end(), buf) -
					lut_trigger_sources.begin();
			if (pos < lut_trigger_sources.size())
				ui->cmb_trigg_source->setCurrentIndex(pos);
		}
		trigger_ab_enabled_update(triggerA_en, triggerB_en);

		trigg_delay_write_hardware(0.0);
	}
}

void TriggerSettings::ui_reconf_on_triggerA_mode_changed(int index)
{
	bool on;

	switch (index) {
	case 0:
	case 1:
		on = (index == 0);
		ui->cmb_triggA_cond->setEnabled(on);
		ui->lbl_triggA_cond->setEnabled(on);
		ui_triggerAlevel->setEnabled(on);
		ui_triggerAHyst->setEnabled(on);
		ui->lbl_triggA_ext_cond->setDisabled(on);
		ui->cmb_triggA_ext_cond->setDisabled(on);
		break;
	default:
		ui->cmb_triggA_cond->setEnabled(true);
		ui->lbl_triggA_cond->setEnabled(true);
		ui_triggerAlevel->setEnabled(true);
		ui_triggerAHyst->setEnabled(true);
		ui->lbl_triggA_ext_cond->setEnabled(true);
		ui->cmb_triggA_ext_cond->setEnabled(true);
		break;
	}
}

void TriggerSettings::ui_reconf_on_triggerB_mode_changed(int index)
{
	bool on;

	switch (index) {
	case 0:
	case 1:
		on = (index == 0);
		ui->cmb_triggB_cond->setEnabled(on);
		ui->lbl_triggB_cond->setEnabled(on);
		ui_triggerBlevel->setEnabled(on);
		ui_triggerBHyst->setEnabled(on);
		ui->lbl_triggB_ext_cond->setDisabled(on);
		ui->cmb_triggB_ext_cond->setDisabled(on);
		break;
	default:
		ui->cmb_triggB_cond->setEnabled(true);
		ui->lbl_triggB_cond->setEnabled(true);
		ui_triggerBlevel->setEnabled(true);
		ui_triggerBHyst->setEnabled(true);
		ui->lbl_triggB_ext_cond->setEnabled(true);
		ui->cmb_triggB_ext_cond->setEnabled(true);
		break;
	}
}

void TriggerSettings::ui_reconf_on_triggerA_cond_changed(int index)
{
	if (index == ui->cmb_triggA_cond->count() - 1) {
		ui->trigg_A_extern_en->setChecked(false);
		ui->trigg_A_extern_en->setEnabled(false);
		ui->lbl_external_a_en->setEnabled(false);
	} else {
		ui->trigg_A_extern_en->setEnabled(true);
		ui->lbl_external_a_en->setEnabled(true);
	}
}

void TriggerSettings::ui_reconf_on_triggerB_cond_changed(int index)
{
	if (index == ui->cmb_triggA_cond->count() - 1) {
		ui->trigg_B_extern_en->setChecked(false);
		ui->trigg_B_extern_en->setEnabled(false);
		ui->lbl_external_b_en->setEnabled(false);
	} else {
		ui->trigg_B_extern_en->setEnabled(true);
		ui->lbl_external_b_en->setEnabled(true);
	}
}

void TriggerSettings::ui_reconf_on_triggerA_extern_changed(bool checked)
{
	QString trigger_type;
	if (checked)
		trigger_type = "Digital(External)";
	else
		trigger_type = "Analog";

	int i = ui->cmb_trigg_A->findText(trigger_type, Qt::MatchExactly);
	ui->cmb_trigg_A->setCurrentIndex(i);

	ui->cmb_trigg_A->setEnabled(checked);
	ui->lbl_trigg_A->setEnabled(checked);
	ui->cmb_triggA_ext_cond->setEnabled(checked);
	ui->lbl_triggA_ext_cond->setEnabled(checked);
}

void TriggerSettings::ui_reconf_on_triggerB_extern_changed(bool checked)
{
	QString trigger_type;
	if (checked)
		trigger_type = "Digital(External)";
	else
		trigger_type = "Analog";

	int i = ui->cmb_trigg_B->findText(trigger_type, Qt::MatchExactly);
	ui->cmb_trigg_B->setCurrentIndex(i);

	ui->cmb_trigg_B->setEnabled(checked);
	ui->lbl_trigg_B->setEnabled(checked);
	ui->cmb_triggB_ext_cond->setEnabled(checked);
	ui->lbl_triggB_ext_cond->setEnabled(checked);
}

void TriggerSettings::trigger_ab_enabled_update(bool &a_en, bool &b_en)
{
	int index = ui->cmb_trigg_source->currentIndex();

	a_en = true;
	b_en = true;
	if (index == 0)
		b_en = false;
	else if (index == 1)
		a_en = false;
}

void TriggerSettings::trigg_delay_write_hardware(double delay)
{
	int sampleWorthOfTime = delay * 1e9  // Convert from seconds to ns
					/ 10; // Each sample is 10 ns distant from each other.

	/* Sync the hardware trigger delay with the trigger point of the plot */
	sampleWorthOfTime -= plot_num_samples / 2;

	QString s = QString::number(sampleWorthOfTime);
	if (this->timeTrigger) {
		iio_channel_attr_write(this->timeTrigger, "delay",
				      s.toLocal8Bit().QByteArray::constData());
	}
}

void TriggerSettings::triggA_level_write_hardware(double value)
{
	if (!this->trigger0)
		return;

	int rawValue = adc_sample_conv::convVoltsToSample(value);
	QString s = QString::number(rawValue);

	iio_channel_attr_write(this->trigger0, "trigger_level",
				s.toLocal8Bit().QByteArray::constData());
}

void TriggerSettings::triggB_level_write_hardware(double value)
{
	if (!this->trigger1)
		return;

	int rawValue = adc_sample_conv::convVoltsToSample(value);
	QString s = QString::number(rawValue);

	iio_channel_attr_write(this->trigger1, "trigger_level",
				s.toLocal8Bit().QByteArray::constData());
}

void adiscope::TriggerSettings::on_btn_noise_reject_toggled(bool checked)
{
	if (checked) {
		ui_triggerAHyst->setEnabled(true);
		ui_triggerBHyst->setEnabled(true);

		ui_triggerAHyst->setValue(hystA_last_val);
		ui_triggerBHyst->setValue(hystB_last_val);
	} else {
		hystA_last_val = ui_triggerAHyst->value();
		hystB_last_val = ui_triggerBHyst->value();

		ui_triggerAHyst->setValue(0);
		ui_triggerBHyst->setValue(0);

		ui_triggerAHyst->setEnabled(false);
		ui_triggerBHyst->setEnabled(false);
	}
}

void TriggerSettings::setPlotNumSamples(int numSamples)
{
	plot_num_samples = numSamples;
}
