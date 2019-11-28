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

#ifndef TRIGGER_SETTINGS_HPP
#define TRIGGER_SETTINGS_HPP

#include "hardware_trigger.hpp"

#include <QWidget>
#include <string>
#include <vector>
#include <map>
#include <memory>

extern "C" {
	struct iio_context;
	struct iio_device;
	struct iio_channel;
}

namespace Ui {
	class TriggerSettings;
}

namespace adiscope {
	class GenericAdc;
	class Oscilloscope_API;
	class PositionSpinButton;
}

namespace adiscope {
	class TriggerSettings : public QWidget
	{
		friend class Oscilloscope_API;

		Q_OBJECT

	public:

		enum TriggerMode {
			NORMAL,
			AUTO
		};

		explicit TriggerSettings(std::shared_ptr<GenericAdc> adc,
					QWidget *parent = nullptr);
		~TriggerSettings();

		int currentChannel() const;
		bool analogEnabled() const;
		bool digitalEnabled() const;
		bool externalOutEnabled() const;
		double level() const;
		double dcLevel() const;
		double hysteresis() const;
		bool triggerIsArmed() const;
		enum TriggerMode triggerMode() const;
		long long triggerDelay() const;
		void setDcLevelCoupled(double);
		void setAcCoupled(bool, int);

		void setChannelAttenuation(double value);

	Q_SIGNALS:
		void sourceChanged(int);
		void levelChanged(double);
		void analogTriggerEnabled(bool);
		void triggerModeChanged(int);

	public Q_SLOTS:
		void setTriggerDelay(long long);
		void setDaisyChainCompensation();
		void setTriggerLevel(double);
		void setTriggerHysteresis(double);
		void setTriggerLevelRange(int chn,
			const QPair<double, double>& range);
		void setTriggerHystRange(int chn,
			const QPair<double, double>& range);
		void setTriggerEnable(bool);
		void setTriggerSource(int);
		void setTriggerLevelStep(int chn, double step);
		void setTriggerHystStep(int chn, double step);
		void autoTriggerDisable();
		void autoTriggerEnable();
		void updateHwVoltLevels(int chnIdx);
		void setAdcRunningState(bool on);
		void onSpinboxTriggerLevelChanged(double);

	private Q_SLOTS:
		void onSpinboxTriggerHystChanged(double);
		void on_cmb_condition_currentIndexChanged(int);
		void on_cmb_extern_condition_currentIndexChanged(int);
		void on_intern_en_toggled(bool);
		void on_cmb_source_currentIndexChanged(int);
		void on_extern_en_toggled(bool);
		void on_extern_to_en_toggled(bool);
		void on_cmb_analog_extern_currentIndexChanged(int);
		void on_cmb_extern_to_src_currentIndexChanged(int);
		void on_btnAuto_toggled(bool);

		void on_cmb_extern_src_currentIndexChanged(int idx);
		void on_spin_daisyChain_valueChanged(int arg1);

	private:
		void setupExternalTriggerDirection();
		void delay_hw_write(long long delay);
		void level_hw_write(double level);
		void hysteresis_hw_write(double level);
		void analog_cond_hw_write(int cond);
		void digital_cond_hw_write(int cond);
		void mode_hw_write(int mode);
		void source_hw_write(int source);

		void ui_reconf_on_intern_toggled(bool);
		void ui_reconf_on_extern_toggled(bool);
		void write_ui_settings_to_hawrdware();
		void trigg_level_write_hardware(int chn, double value);
		HardwareTrigger::mode determineTriggerMode(bool intern_checked,
			bool extern_checked) const;
		void enableExternalTriggerOut(bool);

	private:
		struct trigg_channel_config;

		Ui::TriggerSettings *ui;
		std::shared_ptr<GenericAdc> adc;
		std::shared_ptr<HardwareTrigger> trigger;

		static const std::vector<std::pair<std::string, HardwareTrigger::out_select>> externalTriggerOutMapping;

		QList<trigg_channel_config> trigg_configs;
		PositionSpinButton *trigger_level;
		PositionSpinButton *trigger_hysteresis;
		int current_channel;
		bool temporarily_disabled;
		bool trigger_auto_mode;
		long long trigger_raw_delay;
		long long daisyChainCompensation;
		bool adc_running;
		bool m_ac_coupled;
		double m_displayScale;
	};

}

#endif /* TRIGGER_SETTINGS_HPP */
