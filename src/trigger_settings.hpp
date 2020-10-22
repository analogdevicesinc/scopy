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

#include <QWidget>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <libm2k/enums.hpp>

namespace Ui {
	class TriggerSettings;
}

namespace adiscope {
	class Oscilloscope_API;
	class PositionSpinButton;
}

namespace libm2k {
	class M2kHardwareTrigger;
	namespace analog {
		class M2kAnalogIn;
	}
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

		explicit TriggerSettings(libm2k::analog::M2kAnalogIn* libm2k_adc,
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

		void enableMixedSignalView();
		void disableMixedSignalView();

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
			const std::pair<double, double>& range);
		void setTriggerHystRange(int chn,
			const std::pair<double, double>& range);
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
		void on_btnTrigger_toggled(bool);

		void on_cmb_extern_src_currentIndexChanged(int idx);
		void on_spin_daisyChain_valueChanged(int arg1);

	private:
		void initInstrumentStrings();
		void writeHwDelay(long long delay);
		void writeHwLevel(double level);
		void writeHwHysteresis(double level);
		void writeHwAnalogCondition(int cond);
		void writeHwDigitalCondition(int cond);
		void writeHwMode(int mode);
		void writeHwSource(int source);

		void ui_reconf_on_intern_toggled(bool);
		void ui_reconf_on_extern_toggled(bool);
		void write_ui_settings_to_hardware();
		void trigg_level_write_hardware(int chn, double value);
		libm2k::M2K_TRIGGER_MODE determineTriggerMode(bool intern_checked,
			bool extern_checked) const;
		void enableExternalTriggerOut(bool);

	private:
		struct trigg_channel_config;

		Ui::TriggerSettings *ui;
		libm2k::analog::M2kAnalogIn* m_m2k_adc;
		libm2k::M2kHardwareTrigger* m_trigger;
		bool m_trigger_in;

		std::vector<std::pair<QString, libm2k::M2K_TRIGGER_OUT_SELECT>> externalTriggerOutMapping;

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
		bool m_has_external_trigger_out;
		double m_displayScale;
		void setTriggerIn(bool bo);
	};

}

#endif /* TRIGGER_SETTINGS_HPP */
