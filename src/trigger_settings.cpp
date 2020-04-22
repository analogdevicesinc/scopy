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
#include "osc_adc.h"
#include "scroll_filter.hpp"
#include "spinbox_a.hpp"

#include "ui_trigger_settings.h"

#include <iio.h>

#include <QDebug>

using namespace adiscope;
using namespace std;

struct TriggerSettings::trigg_channel_config
{
	double level_min;
	double level_max;
	double level_step;
	double level_val;
	double hyst_min;
	double hyst_max;
	double hyst_step;
	double hyst_val;
	double dc_level;
};

const std::vector<std::pair<std::string, HardwareTrigger::out_select>> TriggerSettings::externalTriggerOutMapping = {
	//{"None", HardwareTrigger::out_select::sw_trigger},
	{"Forward Trigger In", HardwareTrigger::out_select::trigger_i_swap_channel},
	{"Oscilloscope", HardwareTrigger::out_select::trigger_adc},
	{"Logic Analyzer", HardwareTrigger::out_select::trigger_in},
};

TriggerSettings::TriggerSettings(std::shared_ptr<GenericAdc> adc, QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::TriggerSettings)
	, adc(adc)
	, trigger(adc->getTrigger())
	, current_channel(0)
	, temporarily_disabled(false)
	, adc_running(false)
	, trigger_raw_delay(0)
	, daisyChainCompensation(0)
{
	ui->setupUi(this);

	for (uint i = 0; i < trigger->numChannels(); i++) {
		struct trigg_channel_config config = {};
		trigg_configs.push_back(config);
	}

	trigger_level = new PositionSpinButton({{"μVolts", 1e-6}, {"mVolts", 1e-3}, {"Volts", 1e0}}, "Level", 0.0, 0.0,
					       true, false, this);

	trigger_hysteresis = new PositionSpinButton({{"μVolts", 1e-6}, {"mVolts", 1e-3}, {"Volts", 1e0}}, "Hysteresis",
						    0.0, 0.0, true, false, this);

	ui->controlsLayout->addWidget(trigger_level);
	ui->controlsLayout->addWidget(trigger_hysteresis);

	// Populate UI source comboboxes with the available channels
	for (uint i = 0; i < trigger->numChannels(); i++) {
		ui->cmb_source->addItem(QString("Channel %1").arg(i + 1));
	}

	trigger_auto_mode = ui->btnTrigger->isChecked();

	auto m2k_adc = dynamic_pointer_cast<M2kAdc>(adc);
	if (m2k_adc) {
		auto adc_range = m2k_adc->inputRange(m2k_adc->chnHwGainMode(current_channel));
		auto hyst_range = QPair<double, double>(0, adc_range.second / 10);
		setTriggerLevelRange(current_channel, adc_range);
		setTriggerHystRange(current_channel, hyst_range);
		if (trigger->hasExternalTriggerIn())
			trigger->setExternalDirection(0, HardwareTrigger::direction::TRIGGER_INPUT);
		if (trigger->hasExternalTriggerOut())
			trigger->setExternalDirection(1, HardwareTrigger::direction::TRIGGER_OUT);
	}

	connect(trigger_level, SIGNAL(valueChanged(double)), SLOT(onSpinboxTriggerLevelChanged(double)));
	connect(trigger_hysteresis, SIGNAL(valueChanged(double)), SLOT(onSpinboxTriggerHystChanged(double)));

	connect(ui->btnTrigger, SIGNAL(clicked()), this, SLOT(autoTriggerEnable()));

	// Default GUI settings
	ui->cmb_source->setCurrentIndex(0);
	on_cmb_source_currentIndexChanged(0);
	ui->intern_en->setChecked(true);
	ui->extern_en->setChecked(false);
	on_extern_en_toggled(false);
	ui->cmb_condition->setCurrentIndex(0);
	on_cmb_condition_currentIndexChanged(0);
	ui->cmb_extern_condition->setCurrentIndex(0);
	on_cmb_extern_condition_currentIndexChanged(0);
	ui->cmb_analog_extern->setCurrentIndex(0);
	on_cmb_analog_extern_currentIndexChanged(0);

	ui->label_daisyChain->setVisible(false);
	ui->spin_daisyChain->setVisible(false);

	if (trigger->hasExternalTriggerIn())
		ui->cmb_extern_src->addItem("External Trigger In");
	if (trigger->hasCrossInstrumentTrigger())
		ui->cmb_extern_src->addItem("Logic Analyzer");

	if (trigger->hasExternalTriggerOut()) {
		for (const auto& val : externalTriggerOutMapping) {
			ui->cmb_extern_to_src->addItem(QString::fromStdString(val.first));
			connect(ui->extern_to_en, SIGNAL(toggled(bool)), ui->cmb_extern_to_src, SLOT(setEnabled(bool)));
		}
	} else {
		ui->cmb_extern_to_src->addItem("None");
		ui->cmb_extern_to_src->setEnabled(false);
		ui->extern_to_en->setEnabled(false);
	}

	trigger_level->setValue(0);
	m_ac_coupled = false;
	trigger_hysteresis->setValue(50e-3);
	MouseWheelWidgetGuard* wheelEventGuard = new MouseWheelWidgetGuard(this);
	wheelEventGuard->installEventRecursively(this);
}

TriggerSettings::~TriggerSettings()
{
	/* Restore the trigger setting if we're in auto-trigger mode */
	autoTriggerEnable();

	delete ui;
}

int TriggerSettings::currentChannel() const { return current_channel; }

bool TriggerSettings::analogEnabled() const { return ui->intern_en->isChecked(); }

bool TriggerSettings::digitalEnabled() const { return ui->extern_en->isChecked(); }

bool TriggerSettings::externalOutEnabled() const { return ui->extern_to_en->isChecked(); }

double TriggerSettings::level() const { return trigger_level->value(); }

long long TriggerSettings::triggerDelay() const { return trigger_raw_delay; }

double TriggerSettings::dcLevel() const { return trigg_configs[current_channel].dc_level; }

void TriggerSettings::setDaisyChainCompensation()
{
	const long long DELAY_PER_DEVICE = 23;
	if (ui->extern_en->isChecked())
		daisyChainCompensation =
			ui->spin_daisyChain->value() * DELAY_PER_DEVICE; // if not enabled -> compensation 0
	else
		daisyChainCompensation = 0;
}

void TriggerSettings::setTriggerDelay(long long raw_delay)
{
	static long long oldCompensation = 0;
	setDaisyChainCompensation();
	if ((trigger_raw_delay + oldCompensation) != (raw_delay + daisyChainCompensation)) {
		trigger_raw_delay = raw_delay;
		oldCompensation = daisyChainCompensation;
		delay_hw_write(raw_delay + daisyChainCompensation);
	}
}

void TriggerSettings::setAcCoupled(bool coupled, int chnIdx)
{
	if (chnIdx == current_channel) {
		m_ac_coupled = coupled;
	}
}

void TriggerSettings::setChannelAttenuation(double value)
{
	m_displayScale = value;
	trigger_hysteresis->setDisplayScale(value);
	trigger_level->setDisplayScale(value);
}

void TriggerSettings::setDcLevelCoupled(double value)
{
	if (trigg_configs[current_channel].dc_level != value) {
		trigg_configs[current_channel].dc_level = value;
	}
}

void TriggerSettings::setTriggerLevel(double level)
{
	double current_level = trigger_level->value();
	if (current_level != level) {
		trigger_level->setValue(level);
		trigg_configs[current_channel].level_val = level;
	}
}

void TriggerSettings::setTriggerHysteresis(double hyst)
{
	double current_hyst = trigger_hysteresis->value();

	if (current_hyst != hyst) {
		trigger_hysteresis->setValue(hyst);
		trigg_configs[current_channel].hyst_val = hyst;
	}
}

void TriggerSettings::on_cmb_source_currentIndexChanged(int index)
{
	current_channel = index;
	m_ac_coupled = false;

	trigger_level->setMinValue(trigg_configs[index].level_min);
	trigger_level->setMaxValue(trigg_configs[index].level_max);
	trigger_level->setStep(trigg_configs[index].level_step);
	trigger_hysteresis->setMinValue(trigg_configs[index].hyst_min);
	trigger_hysteresis->setMaxValue(trigg_configs[index].hyst_max);
	trigger_hysteresis->setStep(trigg_configs[index].hyst_step);

	if (adc_running)
		write_ui_settings_to_hawrdware();

	Q_EMIT sourceChanged(index);
}

void TriggerSettings::onSpinboxTriggerLevelChanged(double value)
{
	level_hw_write(value);
	trigg_configs[current_channel].level_val = value;
	Q_EMIT levelChanged(value);
}

void TriggerSettings::onSpinboxTriggerHystChanged(double value)
{
	hysteresis_hw_write(value);
	trigg_configs[current_channel].hyst_val = value;
}

void TriggerSettings::on_cmb_condition_currentIndexChanged(int index) { analog_cond_hw_write(index); }

void TriggerSettings::on_cmb_extern_condition_currentIndexChanged(int index) { digital_cond_hw_write(index); }

void TriggerSettings::on_cmb_extern_to_src_currentIndexChanged(int index)
{
	enableExternalTriggerOut(ui->extern_to_en->isChecked());
}

void TriggerSettings::on_intern_en_toggled(bool checked)
{
	write_ui_settings_to_hawrdware();

	ui_reconf_on_intern_toggled(checked);

	Q_EMIT(analogTriggerEnabled(checked));
}

void TriggerSettings::on_extern_en_toggled(bool checked)
{
	write_ui_settings_to_hawrdware();
	setTriggerDelay(trigger_raw_delay);
	ui_reconf_on_extern_toggled(checked);
}

void TriggerSettings::on_extern_to_en_toggled(bool checked)
{
	enableExternalTriggerOut(checked);
	ui->external_to_controls->setEnabled(checked);
	setTriggerDelay(trigger_raw_delay);
}

HardwareTrigger::mode TriggerSettings::determineTriggerMode(bool intern_checked, bool extern_checked) const
{
	HardwareTrigger::mode mode;

	if (intern_checked) {
		if (extern_checked) {
			int n = static_cast<int>(HardwareTrigger::DIGITAL) + 1;
			int i = ui->cmb_analog_extern->currentIndex();

			mode = static_cast<HardwareTrigger::mode>(n + i);
		} else {
			mode = HardwareTrigger::ANALOG;
		}
	} else {
		if (extern_checked) {
			mode = HardwareTrigger::DIGITAL;
		} else {
			mode = HardwareTrigger::ALWAYS;
		}
	}

	return mode;
}

void TriggerSettings::enableExternalTriggerOut(bool enabled)
{
	const int TRIGGER_OUT_PIN = 1;

	if (enabled)
		trigger->setExternalOutSelect(TRIGGER_OUT_PIN,
					      externalTriggerOutMapping[ui->cmb_extern_to_src->currentIndex()].second);
	else
		trigger->setExternalOutSelect(TRIGGER_OUT_PIN, HardwareTrigger::out_select::sw_trigger);
}

void TriggerSettings::ui_reconf_on_intern_toggled(bool checked)
{
	ui->analog_controls->setEnabled(checked);

	bool analog_and_ext = checked && ui->extern_en->isChecked();
	ui->lbl_analog_extern->setEnabled(analog_and_ext);
	ui->cmb_analog_extern->setEnabled(analog_and_ext);
}

void TriggerSettings::ui_reconf_on_extern_toggled(bool checked)
{
	ui->digital_controls->setEnabled(checked);

	bool analog_and_ext = checked && ui->intern_en->isChecked();
	ui->lbl_analog_extern->setEnabled(analog_and_ext);
	ui->cmb_analog_extern->setEnabled(analog_and_ext);
}

void TriggerSettings::on_cmb_analog_extern_currentIndexChanged(int index)
{
	if (adc_running) {
		HardwareTrigger::mode mode;
		int n = static_cast<int>(HardwareTrigger::DIGITAL) + 1;

		mode = static_cast<HardwareTrigger::mode>(n + index);
		try {
			trigger->setTriggerMode(current_channel, mode);
		} catch (std::exception& e) {
			qDebug() << e.what();
		}
	}
}

void TriggerSettings::autoTriggerDisable()
{
	if (!ui->btnTrigger->isChecked()) {
		mode_hw_write(HardwareTrigger::ALWAYS);

		temporarily_disabled = true;
	}
}

void TriggerSettings::autoTriggerEnable()
{
	if (!ui->btnTrigger->isChecked()) {
		if (temporarily_disabled) {
			mode_hw_write(determineTriggerMode(ui->intern_en->isChecked(), ui->extern_en->isChecked()));

			temporarily_disabled = false;
		}
	}
}

bool TriggerSettings::triggerIsArmed() const { return ui->intern_en->isChecked() || ui->extern_en->isChecked(); }

void TriggerSettings::on_btnAuto_toggled(bool checked)
{
	trigger_auto_mode = checked;
	int mode = checked ? AUTO : NORMAL;

	Q_EMIT triggerModeChanged(mode);
}

TriggerSettings::TriggerMode TriggerSettings::triggerMode() const
{
	return ui->btnTrigger->isChecked() ? AUTO : NORMAL;
}

void TriggerSettings::updateHwVoltLevels(int chnIdx)
{
	try {
		double level = trigg_configs[chnIdx].level_val;
		if (m_ac_coupled) {
			level = level + trigg_configs[current_channel].dc_level;
		}
		int rawValue = (int)adc->convVoltsToSample(chnIdx, level);
		trigger->setLevel(chnIdx, rawValue);

		rawValue = (int)adc->convVoltsDiffToSampleDiff(chnIdx, trigg_configs[chnIdx].hyst_val);
		trigger->setHysteresis(chnIdx, rawValue);
	} catch (std::exception& e) {
		qDebug() << e.what();
	}
}

void TriggerSettings::setTriggerLevelRange(int chn, const QPair<double, double>& range)
{
	trigg_configs[chn].level_min = range.first;
	trigg_configs[chn].level_max = range.second;

	if (current_channel == chn) {
		trigger_level->setMinValue(range.first);
		trigger_level->setMaxValue(range.second);
	}
}

void TriggerSettings::setTriggerHystRange(int chn, const QPair<double, double>& range)
{
	trigg_configs[chn].hyst_min = range.first;
	trigg_configs[chn].hyst_max = range.second;

	if (current_channel == chn) {
		trigger_hysteresis->setMinValue(range.first);
		trigger_hysteresis->setMaxValue(range.second);
	}
}

void TriggerSettings::setTriggerEnable(bool en)
{
	ui->intern_en->setChecked(en);
	on_intern_en_toggled(en);
}

void TriggerSettings::setTriggerSource(int ch)
{
	ui->cmb_source->setCurrentIndex(ch);
	on_cmb_source_currentIndexChanged(ch);
}

void TriggerSettings::setTriggerLevelStep(int chn, double step)
{
	trigg_configs[chn].level_step = step;

	if (current_channel == chn) {
		trigger_level->setStep(step);
	}
}

void TriggerSettings::setTriggerHystStep(int chn, double step)
{
	trigg_configs[chn].hyst_step = step;

	if (current_channel == chn) {
		trigger_hysteresis->setStep(step);
	}
}

void TriggerSettings::setAdcRunningState(bool on)
{
	adc_running = on;

	if (on) {
		write_ui_settings_to_hawrdware();
	}
}

void TriggerSettings::write_ui_settings_to_hawrdware()
{
	int extern_cnd;

	setDaisyChainCompensation();
	source_hw_write(ui->cmb_source->currentIndex());
	mode_hw_write(determineTriggerMode(ui->intern_en->isChecked(), ui->extern_en->isChecked()));
	analog_cond_hw_write(ui->cmb_condition->currentIndex());
	if (ui->cmb_extern_src->currentIndex() == 0)
		extern_cnd = ui->cmb_extern_condition->currentIndex();
	else
		extern_cnd = HardwareTrigger::FALLING_EDGE; // when logic analyzer is
							    // selected set condition
							    // to falling edge
	digital_cond_hw_write(extern_cnd);
	level_hw_write(trigger_level->value());
	hysteresis_hw_write(trigger_hysteresis->value());
	delay_hw_write(trigger_raw_delay + daisyChainCompensation);
}

void TriggerSettings::setupExternalTriggerDirection()
{
	try {
		trigger->setExternalDirection(0, HardwareTrigger::direction::TRIGGER_INPUT);
		trigger->setExternalDirection(1, HardwareTrigger::direction::TRIGGER_OUT);
	} catch (std::exception& e) {
		qDebug() << e.what();
	}
}

void TriggerSettings::delay_hw_write(long long raw_delay)
{
	if (adc_running) {
		try {
			trigger->setDelay(raw_delay);
		} catch (std::exception& e) {
			qDebug() << e.what();
		}
	}
}

void TriggerSettings::level_hw_write(double level)
{
	if (adc_running) {
		if (m_ac_coupled) {
			level = level + trigg_configs[current_channel].dc_level;
		}
		int rawValue = (int)adc->convVoltsToSample(current_channel, level);

		try {
			trigger->setLevel(current_channel, rawValue);
		} catch (std::exception& e) {
			qDebug() << e.what();
		}
	}
}

void TriggerSettings::hysteresis_hw_write(double level)
{
	if (adc_running) {
		int rawValue = (int)adc->convVoltsDiffToSampleDiff(current_channel, level);

		try {
			trigger->setHysteresis(current_channel, rawValue);
		} catch (std::exception& e) {
			qDebug() << e.what();
		}
	}
}

void TriggerSettings::analog_cond_hw_write(int cond)
{
	if (adc_running) {
		HardwareTrigger::condition t_cond = static_cast<HardwareTrigger::condition>(cond);

		try {
			trigger->setAnalogCondition(current_channel, t_cond);
		} catch (std::exception& e) {
			qDebug() << e.what();
		}
	}
}

void TriggerSettings::digital_cond_hw_write(int cond)
{
	if (adc_running) {
		HardwareTrigger::condition t_cond = static_cast<HardwareTrigger::condition>(cond);

		try {
			trigger->setDigitalCondition(current_channel, t_cond);
		} catch (std::exception& e) {
			qDebug() << e.what();
		}
	}
}

void TriggerSettings::mode_hw_write(int mode)
{
	if (adc_running) {
		try {
			trigger->setTriggerMode(current_channel, static_cast<HardwareTrigger::mode>(mode));
		} catch (std::exception& e) {
			qDebug() << e.what();
		}
	}
}

void TriggerSettings::on_cmb_extern_src_currentIndexChanged(int idx)
{
	// set trigger in when both logic analyzer src and extern trig is on
	trigger->setTriggerIn(idx && ui->extern_en->isChecked());
	ui->cmb_extern_condition->setEnabled(idx == 0);
	if (adc_running)
		write_ui_settings_to_hawrdware();
}

void TriggerSettings::source_hw_write(int source)
{
	if (adc_running) {
		try {
			trigger->setSourceChannel(source,
						  ui->intern_en->isChecked(), // analog trigger on
						  (ui->extern_en->isChecked() &&
						   ui->cmb_extern_src->currentIndex() == 0)); // extern trigger on & ext
											      // trigger in
		} catch (std::exception& e) {
			qDebug() << e.what();
		}
	}
}

void adiscope::TriggerSettings::on_spin_daisyChain_valueChanged(int arg1) { setTriggerDelay(trigger_raw_delay); }
