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

#ifndef TRIGGER_SETTINGS_HPP
#define TRIGGER_SETTINGS_HPP

#include <QWidget>
#include <string>

extern "C" {
	struct iio_context;
	struct iio_device;
	struct iio_channel;
}

namespace Ui {
	class TriggerSettings;
}

namespace adiscope {
	class OscADC;
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

		explicit TriggerSettings(struct iio_context *ctx,
					const OscADC& adc,
					QWidget *parent = nullptr);
		~TriggerSettings();

		double levelA_value();
		double levelB_value();
		bool levelA_enabled();
		bool levelB_enabled();
		bool triggerIsArmed() const;
		enum TriggerMode triggerMode() const;
		long long triggerDelay() const;

	Q_SIGNALS:
		void levelAChanged(double);
		void levelBChanged(double);
		void triggerAenabled(bool);
		void triggerBenabled(bool);
		void triggerModeChanged(int);

	public Q_SLOTS:
		void setTriggerDelay(long long);
		void setTriggerLevelA(double);
		void setTriggerLevelB(double);

		void autoTriggerDisable();
		void autoTriggerEnable();

	private Q_SLOTS:
		void on_cmb_trigg_source_currentIndexChanged(int);
		void on_cmb_trigg_A_currentIndexChanged(int);
		void on_cmb_trigg_B_currentIndexChanged(int);
		void onSpinboxTriggerAlevelChanged(double);
		void onSpinboxTriggerBlevelChanged(double);
		void onSpinboxTriggerAhystChanged(double);
		void onSpinboxTriggerBhystChanged(double);
		void on_cmb_triggA_cond_currentIndexChanged(int);
		void on_cmb_triggB_cond_currentIndexChanged(int);
		void on_cmb_triggA_ext_cond_currentIndexChanged(int);
		void on_cmb_triggB_ext_cond_currentIndexChanged(int);
		void on_trigg_A_extern_en_toggled(bool);
		void on_trigg_B_extern_en_toggled(bool);
		void on_btn_noise_reject_toggled(bool);
		void on_btnAuto_toggled(bool);

	private:
		void trigger_all_widgets_update();
		void trigger_ab_enabled_update(bool &a_en, bool &b_en);

		void ui_reconf_on_triggerA_mode_changed(int);
		void ui_reconf_on_triggerB_mode_changed(int);
		void ui_reconf_on_triggerA_cond_changed(int);
		void ui_reconf_on_triggerB_cond_changed(int);
		void ui_reconf_on_triggerA_extern_changed(bool);
		void ui_reconf_on_triggerB_extern_changed(bool);

		void trigg_delay_write_hardware(int raw_delay);
		void triggA_level_write_hardware(double value);
		void triggB_level_write_hardware(double value);

	private:
		Ui::TriggerSettings *ui;
		PositionSpinButton *ui_triggerAlevel;
		PositionSpinButton *ui_triggerBlevel;
		PositionSpinButton *ui_triggerAHyst;
		PositionSpinButton *ui_triggerBHyst;
		PositionSpinButton *ui_triggerHoldoff;

		const OscADC& osc_adc;

		struct iio_channel *trigger0;
		struct iio_channel *trigger1;
		struct iio_channel *digitalTrigger0;
		struct iio_channel *digitalTrigger1;
		struct iio_channel *timeTrigger;
		struct iio_channel *trigger0Mode;
		struct iio_channel *trigger1Mode;
		static std::vector<std::string> lut_trigger_sources;
		static std::vector<std::string> lut_triggerX_types;
		static std::vector<std::string> lut_analog_trigger_conditions;
		static std::vector<std::string> lut_digital_trigger_conditions;

		bool triggerA_en;
		bool triggerB_en;
		bool temporarily_disabled;
		bool trigger_auto_mode;

		long long trigger_raw_delay;
		double hystA_last_val;
		double hystB_last_val;
	};

}

#endif /* TRIGGER_SETTINGS_HPP */
