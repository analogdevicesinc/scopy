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

#include <libm2k/context.hpp>
#include <libm2k/m2k.hpp>
#include <libm2k/m2kexceptions.hpp>
#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/m2khardwaretrigger.hpp>
#include <libm2k/contextbuilder.hpp>

#include "trigger_settings.hpp"
#include "gui/spinbox_a.hpp"
#include "scroll_filter.hpp"

#include "ui_trigger_settings.h"

#include <iio.h>
#include <QDebug>
#include <QTranslator>
#include "scopyExceptionHandler.h"


using namespace adiscope;
using namespace std;
using namespace libm2k;
using namespace libm2k::context;
using namespace libm2k::analog;

struct TriggerSettings::trigg_channel_config {
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

void TriggerSettings::initInstrumentStrings() {
	externalTriggerOutMapping = {
		{tr("Forward Trigger In"), libm2k::SELECT_TRIGGER_IN},
		{tr("Oscilloscope"),  libm2k::SELECT_ANALOG_IN},
		{tr("Logic Analyzer"), libm2k::SELECT_DIGITAL_IN}
	};
}

TriggerSettings::TriggerSettings(M2kAnalogIn* libm2k_adc,
		QWidget *parent) :
	QWidget(parent), ui(new Ui::TriggerSettings),
	m_m2k_adc(libm2k_adc),
	m_trigger(nullptr),
	current_channel(0),
	temporarily_disabled(false),
	adc_running(false),
	trigger_raw_delay(0),
	daisyChainCompensation(0),
	m_trigger_in(false),
	m_has_external_trigger_out(false),
	digital_trigger_was_on(false)
{
	initInstrumentStrings();
	ui->setupUi(this);
	m_trigger = (m_m2k_adc) ? m_m2k_adc->getTrigger() : nullptr;

	for (uint i = 0; i < m_m2k_adc->getNbChannels(); i++) {
		struct trigg_channel_config config = {};
		trigg_configs.push_back(config);
	}

	trigger_level = new PositionSpinButton({
	{tr("μVolts"),1e-6},
	{tr("mVolts"),1e-3},
	{tr("Volts"),1e0}
	}, tr("Level"), 0.0, 0.0, true, false, this);

	trigger_hysteresis = new PositionSpinButton({
	{tr("μVolts"),1e-6},
	{tr("mVolts"),1e-3},
	{tr("Volts"),1e0}
	}, tr("Hysteresis"), 0.0, 0.0, true, false, this);

	ui->controlsLayout->addWidget(trigger_level);
	ui->controlsLayout->addWidget(trigger_hysteresis);

	trigger_auto_mode = ui->btnTrigger->isChecked();

	if (m_m2k_adc) {
		// Populate UI source comboboxes with the available channels
		for (uint i = 0; i < m_m2k_adc->getNbChannels(); i++) {
			ui->cmb_source->addItem(QString("Channel %1").arg(i + 1));
		}

		auto chn = static_cast<libm2k::analog::ANALOG_IN_CHANNEL>(currentChannel());
		libm2k::analog::M2K_RANGE adc_range = m_m2k_adc->getRange(chn);
		std::pair<double, double> adc_range_limits = m_m2k_adc->getRangeLimits(adc_range);
		std::pair<double, double> hyst_range_limits = m_m2k_adc->getHysteresisRange(chn);

		setTriggerLevelRange(currentChannel(), adc_range_limits);
		setTriggerHystRange(currentChannel(), hyst_range_limits);
	}

	connect(trigger_level, SIGNAL(valueChanged(double)),
		SLOT(onSpinboxTriggerLevelChanged(double)));
	connect(trigger_hysteresis, SIGNAL(valueChanged(double)),
		SLOT(onSpinboxTriggerHystChanged(double)));

	connect(ui->btnTrigger, SIGNAL(clicked()), this,
	        SLOT(autoTriggerEnable()));

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

	if (m_trigger->hasExternalTriggerIn()) {
		ui->cmb_extern_src->addItem(tr("External Trigger In"));
	}
	if (m_trigger->hasCrossInstrumentTrigger()) {
		ui->cmb_extern_src->addItem(tr("Logic Analyzer"));
	}

	if (m_trigger->hasExternalTriggerOut()) {
		m_has_external_trigger_out = true;
		for (const auto &val : externalTriggerOutMapping)
		{
			ui->cmb_extern_to_src->addItem(val.first);
			connect(ui->extern_to_en,SIGNAL(toggled(bool)),ui->cmb_extern_to_src,SLOT(setEnabled(bool)));
		}
	} else {
		m_has_external_trigger_out = false;
		ui->cmb_extern_to_src->addItem(tr("None"));
		ui->cmb_extern_to_src->setEnabled(false);
		ui->extern_to_en->setEnabled(false);
	}

	trigger_level->setValue(0);
	m_ac_coupled = false;
	trigger_hysteresis->setValue(50e-3);
	MouseWheelWidgetGuard *wheelEventGuard = new MouseWheelWidgetGuard(this);
	wheelEventGuard->installEventRecursively(this);

	ui->mixedSignalLbl->setVisible(false);
	ui->btnLogicAnalyzerTriggers->setVisible(false);

	connect(ui->btnLogicAnalyzerTriggers, &QPushButton::clicked,
		this, &TriggerSettings::showLogicAnalyzerTriggerSettings);
}

TriggerSettings::~TriggerSettings()
{
	/* Restore the trigger setting if we're in auto-trigger mode */
	autoTriggerEnable();

	delete ui;
}

int TriggerSettings::currentChannel() const
{
	return current_channel;
}

bool TriggerSettings::analogEnabled() const
{
	return ui->intern_en->isChecked();
}

bool TriggerSettings::digitalEnabled() const
{
	return ui->extern_en->isChecked();
}

bool TriggerSettings::externalOutEnabled() const
{
	return ui->extern_to_en->isChecked();
}

double TriggerSettings::level() const
{
	return trigger_level->value();
}

long long TriggerSettings::triggerDelay() const
{
	return trigger_raw_delay;
}

double TriggerSettings::dcLevel() const
{
	return trigg_configs[current_channel].dc_level;
}

void TriggerSettings::setDaisyChainCompensation()
{
	const long long DELAY_PER_DEVICE = 23;
	if(ui->extern_en->isChecked() )
		daisyChainCompensation = ui->spin_daisyChain->value() * DELAY_PER_DEVICE; // if not enabled -> compensation 0
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
		writeHwDelay(raw_delay + daisyChainCompensation);
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

void TriggerSettings::enableMixedSignalView()
{
	ui->extern_en->setDisabled(true);
	ui->extern_to_en->setDisabled(true);
	ui->mixedSignalLbl->setVisible(true);
	ui->btnLogicAnalyzerTriggers->setVisible(true);
	ui->btnLogicAnalyzerTriggers->setEnabled(true);
	digital_trigger_was_on = ui->digital_controls->isEnabled();
	if (digital_trigger_was_on) {
		ui->digital_controls->setDisabled(true);
	}
}

void TriggerSettings::disableMixedSignalView()
{
	ui->extern_en->setEnabled(true);
	ui->extern_to_en->setEnabled(true);
	ui->mixedSignalLbl->setVisible(false);
	ui->btnLogicAnalyzerTriggers->setVisible(false);
	ui->btnLogicAnalyzerTriggers->setDisabled(true);
	if (digital_trigger_was_on) {
		ui->digital_controls->setEnabled(true);
	}
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
		trigg_configs[currentChannel()].hyst_val = hyst;
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
		write_ui_settings_to_hardware();

	Q_EMIT sourceChanged(index);
}

void TriggerSettings::onSpinboxTriggerLevelChanged(double value)
{
	writeHwLevel(value);
	trigg_configs[currentChannel()].level_val = value;
	Q_EMIT levelChanged(value);
}

void TriggerSettings::onSpinboxTriggerHystChanged(double value)
{
	writeHwHysteresis(value);
	trigg_configs[currentChannel()].hyst_val = value;
}

void TriggerSettings::on_cmb_condition_currentIndexChanged(int index)
{
	writeHwAnalogCondition(index);
}

void TriggerSettings::on_cmb_extern_condition_currentIndexChanged(int index)
{
	writeHwDigitalCondition(index);
}

void TriggerSettings::on_cmb_extern_to_src_currentIndexChanged(int index)
{
	enableExternalTriggerOut(ui->extern_to_en->isChecked());
}

void TriggerSettings::on_intern_en_toggled(bool checked)
{
	write_ui_settings_to_hardware();

	ui_reconf_on_intern_toggled(checked);

	Q_EMIT(analogTriggerEnabled(checked));
}

void TriggerSettings::on_extern_en_toggled(bool checked)
{
	write_ui_settings_to_hardware();
	setTriggerDelay(trigger_raw_delay);
	ui_reconf_on_extern_toggled(checked);
}

void TriggerSettings::on_extern_to_en_toggled(bool checked)
{
	enableExternalTriggerOut(checked);
	ui->external_to_controls->setEnabled(checked);
	setTriggerDelay(trigger_raw_delay);
}

libm2k::M2K_TRIGGER_MODE TriggerSettings::determineTriggerMode(bool intern_checked,
			bool extern_checked) const
{
	libm2k::M2K_TRIGGER_MODE mode;

	if (intern_checked) {
		if (extern_checked) {
			int start_idx = static_cast<int>(libm2k::EXTERNAL) + 1;
			int extern_idx = ui->cmb_analog_extern->currentIndex();
			mode = static_cast<libm2k::M2K_TRIGGER_MODE>(start_idx + extern_idx);
		} else {
			mode = libm2k::ANALOG;
		}
	} else {
		if (extern_checked) {
			mode = libm2k::EXTERNAL;
		} else {
			mode = libm2k::ALWAYS;
		}
	}

	return mode;
}

void TriggerSettings::enableExternalTriggerOut(bool enabled)
{
	const int TRIGGER_OUT_PIN = 1;

	if(m_has_external_trigger_out) {
		try {
			if(enabled) {
				m_trigger->setAnalogExternalOutSelect(externalTriggerOutMapping[ui->cmb_extern_to_src->currentIndex()].second);
			} else {
				m_trigger->setAnalogExternalOutSelect(libm2k::SELECT_NONE);
			}
		} catch (libm2k::m2k_exception& e) {
			HANDLE_EXCEPTION(e);
			qDebug() << e.what();
		} catch(std::exception e) {
			HANDLE_EXCEPTION(e);
			qDebug() << e.what();
		}
	}
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
		libm2k::M2K_TRIGGER_MODE mode;
		int start_idx = static_cast<int>(libm2k::EXTERNAL) + 1;
		mode = static_cast<libm2k::M2K_TRIGGER_MODE>(start_idx + index);
		try {
			m_trigger->setAnalogMode(currentChannel(), mode);
		} catch (libm2k::m2k_exception& e){
			HANDLE_EXCEPTION(e);
			qDebug() << e.what();
		}
	}
}

void TriggerSettings::autoTriggerDisable()
{
	if (!ui->btnTrigger->isChecked()) {
		writeHwMode(libm2k::ALWAYS);
		temporarily_disabled = true;
	}
}

void TriggerSettings::autoTriggerEnable()
{
	if (!ui->btnTrigger->isChecked()) {
		if (temporarily_disabled) {
			writeHwMode(determineTriggerMode(ui->intern_en->isChecked(),
							 ui->extern_en->isChecked()));

			temporarily_disabled = false;
		}
	} else {
		writeHwMode(determineTriggerMode(ui->intern_en->isChecked(),
                                                ui->extern_en->isChecked()));
	}
}

bool TriggerSettings::triggerIsArmed() const
{
	return ui->intern_en->isChecked() || ui->extern_en->isChecked();
}

void TriggerSettings::on_btnTrigger_toggled(bool checked)
{
	trigger_auto_mode = checked;
	int mode = checked ? NORMAL : AUTO;

	Q_EMIT triggerModeChanged(mode);
}

TriggerSettings::TriggerMode TriggerSettings::triggerMode() const
{
	return ui->btnTrigger->isChecked() ? NORMAL : AUTO;
}

void TriggerSettings::updateHwVoltLevels(int chnIdx)
{
	try {
		double level = trigg_configs[chnIdx].level_val;
		if (m_ac_coupled) {
			level = level + trigg_configs[current_channel].dc_level;
		}
		m_trigger->setAnalogLevel(chnIdx, level);
		m_trigger->setAnalogHysteresis(chnIdx, trigg_configs[chnIdx].hyst_val);
	}
	catch (libm2k::m2k_exception& e)
	{
		HANDLE_EXCEPTION(e);
		qDebug() << e.what();
	}
}

void TriggerSettings::setTriggerLevelRange(int chn,
	const std::pair<double, double>& range)
{
	trigg_configs[chn].level_min = range.first;
	trigg_configs[chn].level_max = range.second;

	if (current_channel == chn) {
		trigger_level->setMinValue(range.first);
		trigger_level->setMaxValue(range.second);
	}
}

void TriggerSettings::setTriggerHystRange(int chn,
	const std::pair<double, double>& range)
{
	trigg_configs[chn].hyst_min = range.first;
	trigg_configs[chn].hyst_max = range.second;

	if (current_channel == chn) {
		trigger_hysteresis->setMinValue(range.first);
		trigger_hysteresis->setMaxValue(range.second);
	}
}

void TriggerSettings::setTriggerEnable(bool en) {
	ui->intern_en->setChecked(en);
	on_intern_en_toggled(en);
}

void TriggerSettings::setTriggerSource(int ch) {
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
		write_ui_settings_to_hardware();
	}
}

void TriggerSettings::write_ui_settings_to_hardware()
{
	int extern_cnd;

	setDaisyChainCompensation();
	writeHwSource(ui->cmb_source->currentIndex());
	writeHwMode(determineTriggerMode(ui->intern_en->isChecked(),
		ui->extern_en->isChecked()));
	writeHwAnalogCondition(ui->cmb_condition->currentIndex());
	if(ui->cmb_extern_src->currentIndex() == 0) { /* case of External Trigger In */
		extern_cnd = ui->cmb_extern_condition->currentIndex();
	} else {
		/* When logic analyzer is selected set condition to falling edge */
		extern_cnd = libm2k::FALLING_EDGE_DIGITAL;
	}
	writeHwDigitalCondition(extern_cnd);
	writeHwLevel(trigger_level->value());
	writeHwHysteresis(trigger_hysteresis->value());
	writeHwDelay(trigger_raw_delay + daisyChainCompensation);
}

void TriggerSettings::writeHwDelay(long long raw_delay)
{
	if (adc_running) {
		try {
			m_trigger->setAnalogDelay(raw_delay);
		}
		catch (libm2k::m2k_exception& e) {
			HANDLE_EXCEPTION(e);
			qDebug() << e.what();
		}
	}
}

void TriggerSettings::writeHwLevel(double level)
{
	if (adc_running) {
		if (m_ac_coupled) {
			level = level + trigg_configs[current_channel].dc_level;
		}

		try {
			m_trigger->setAnalogLevel(currentChannel(), level);
		}
		catch (libm2k::m2k_exception& e) {
			HANDLE_EXCEPTION(e);
			qDebug() << e.what();
		}
	}
}

void TriggerSettings::writeHwHysteresis(double level)
{
	if (adc_running) {
		try {
			m_trigger->setAnalogHysteresis(currentChannel(), level);
		}
		catch (libm2k::m2k_exception& e) {
			HANDLE_EXCEPTION(e);
			qDebug() << e.what();
		}
	}
}

void TriggerSettings::writeHwAnalogCondition(int cond)
{
	if (adc_running) {
		libm2k::M2K_TRIGGER_CONDITION_ANALOG t_cond =
			static_cast<libm2k::M2K_TRIGGER_CONDITION_ANALOG>(cond);

		try {
			m_trigger->setAnalogCondition(currentChannel(), t_cond);
		}
		catch (libm2k::m2k_exception& e) {
			HANDLE_EXCEPTION(e);
			qDebug() << e.what();
		}
	}
}

void TriggerSettings::writeHwDigitalCondition(int cond)
{
	if (adc_running) {
		libm2k::M2K_TRIGGER_CONDITION_DIGITAL t_cond =
			static_cast<libm2k::M2K_TRIGGER_CONDITION_DIGITAL>(cond);

		try {
			m_trigger->setAnalogExternalCondition(currentChannel(), t_cond);
		}
		catch (libm2k::m2k_exception& e) {
			HANDLE_EXCEPTION(e);
			qDebug() << e.what();
		}
	}
}

void TriggerSettings::writeHwMode(int mode)
{
	if (adc_running) {
		try {
			m_trigger->setAnalogMode(currentChannel(),
				static_cast<libm2k::M2K_TRIGGER_MODE>(mode));
		}
		catch (libm2k::m2k_exception& e)
		{
			HANDLE_EXCEPTION(e);
			qDebug() << e.what();
		}
	}
}

void TriggerSettings::on_cmb_extern_src_currentIndexChanged(int idx)
{
	// mode: digital_OR_analog
	// logic mode (src) a without trigger_in -> for External Trigger In
	// logic_mode (src) a_OR_trigger_in -> for logic analyzer
	// set trigger in when both logic analyzer src and extern trig is on
	setTriggerIn(idx && ui->extern_en->isChecked());
	ui->cmb_extern_condition->setEnabled(idx==0);
	if (adc_running) {
		write_ui_settings_to_hardware();
	}
}

void TriggerSettings::setTriggerIn(bool enable)
{
	m_trigger_in = enable;
}

void TriggerSettings::writeHwSource(int source_chn)
{
	if (adc_running) {
		try {
			libm2k::M2K_TRIGGER_SOURCE_ANALOG source = static_cast<M2K_TRIGGER_SOURCE_ANALOG>(source_chn);
			/* analog trigger on */
			bool intern_checked = ui->intern_en->isChecked();
			/* extern trigger on & ext digital */
			bool extern_digital_checked = (ui->extern_en->isChecked() && ui->cmb_extern_src->currentIndex() == 1);
			if(m_trigger_in) {
				if (!intern_checked) {
				        if (extern_digital_checked) {
				                // digital trigger enabled
                                                source = libm2k::SRC_DIGITAL_IN;
                                        }
				} else {
                                        if (extern_digital_checked) {
                                                // analog + digital trigger
                                                source = static_cast<libm2k::M2K_TRIGGER_SOURCE_ANALOG>(
                                                        (source_chn + 1) + libm2k::SRC_DIGITAL_IN);
                                        }
				}
			}
			m_trigger->setAnalogSource(source);
		}
		catch (libm2k::m2k_exception& e) {
			HANDLE_EXCEPTION(e);
			qDebug() << e.what();
		}
	}
}

void adiscope::TriggerSettings::on_spin_daisyChain_valueChanged(int arg1)
{
    setTriggerDelay(trigger_raw_delay);
}
