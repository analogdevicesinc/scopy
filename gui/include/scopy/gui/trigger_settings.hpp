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

namespace Ui {
class TriggerSettings;
}

namespace scopy {
namespace gui {

class PositionSpinButton;

class TriggerSettings : public QWidget
{
	Q_OBJECT

public:
	enum TriggerMode
	{
		NORMAL,
		AUTO
	};

	explicit TriggerSettings(QWidget* parent = nullptr);
	~TriggerSettings();

private:
	struct trigg_channel_config;

	Ui::TriggerSettings* m_ui;
	bool m_triggerIn;

	bool m_digitalTriggerWasOn;

	QList<trigg_channel_config> m_triggConfigs;
	PositionSpinButton* m_triggerLevel;
	PositionSpinButton* m_triggerHysteresis;
	int m_currentChannel;
	bool m_temporarilyDisabled;
	bool m_triggerAutoMode;
	long long m_triggerRawDelay;
	long long m_daisyChainCompensation;
	bool m_adcRunning;
	bool m_acCoupled;
	bool m_hasExternalTriggerOut;
	double m_displayScale;
};
} // namespace gui
} // namespace scopy

#endif /* TRIGGER_SETTINGS_HPP */
