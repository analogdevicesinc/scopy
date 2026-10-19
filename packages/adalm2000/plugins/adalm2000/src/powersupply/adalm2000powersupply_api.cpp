/*
 * Copyright (c) 2026 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "adalm2000powersupply_api.h"

#include "adalm2000powersupplytool.h"
#include "powersupplymath.h"

#include <QLoggingCategory>
#include <QPushButton>
#include <QSlider>

#include <gui/customSwitch.h>
#include <iio-widgets/iiowidget.h>
#include <pluginbase/scopyjs.h>

Q_LOGGING_CATEGORY(CAT_ADALM2000POWERSUPPLY_API, "Adalm2000PowerSupply_API")

using namespace scopy::adalm2000;

Adalm2000PowerSupply_API::Adalm2000PowerSupply_API(Adalm2000PowerSupplyTool *tool)
	: ApiObject()
	, m_tool(tool)
{}

Adalm2000PowerSupply_API::~Adalm2000PowerSupply_API() { ScopyJS::GetInstance()->unregisterApi(this); }

bool Adalm2000PowerSupply_API::dac1Enabled() const { return m_tool->m_enableBtn[0]->isChecked(); }

void Adalm2000PowerSupply_API::setDac1Enabled(bool en) { m_tool->m_enableBtn[0]->setChecked(en); }

bool Adalm2000PowerSupply_API::dac2Enabled() const { return m_tool->m_enableBtn[1]->isChecked(); }

void Adalm2000PowerSupply_API::setDac2Enabled(bool en)
{
	if(m_tool->m_tracking) {
		qWarning(CAT_ADALM2000POWERSUPPLY_API)
			<< "dac2_enabled is controlled by dac1_enabled in sync (tracking) mode";
		return;
	}
	m_tool->m_enableBtn[1]->setChecked(en);
}

double Adalm2000PowerSupply_API::dac1Value() const { return m_tool->m_setpoint[0]; }

void Adalm2000PowerSupply_API::setDac1Value(double volts)
{
	volts = qBound(ps::POSITIVE_MIN, volts, ps::POSITIVE_MAX);
	if(m_tool->m_voltageWidget[0]) {
		m_tool->m_voltageWidget[0]->writeAsync(QString::number(volts, 'g', 6));
	}
}

double Adalm2000PowerSupply_API::dac2Value() const { return m_tool->m_setpoint[1]; }

void Adalm2000PowerSupply_API::setDac2Value(double volts)
{
	if(m_tool->m_tracking) {
		qWarning(CAT_ADALM2000POWERSUPPLY_API)
			<< "dac2_value is controlled by dac1_value in sync (tracking) mode";
		return;
	}
	volts = qBound(ps::NEGATIVE_MIN, volts, ps::NEGATIVE_MAX);
	if(m_tool->m_voltageWidget[1]) {
		m_tool->m_voltageWidget[1]->writeAsync(QString::number(volts, 'g', 6));
	}
}

bool Adalm2000PowerSupply_API::sync() const { return m_tool->m_tracking; }

void Adalm2000PowerSupply_API::setSync(bool tracking)
{
	// m_modeSwitch: checked = Independent (not tracking), unchecked = Tracking
	m_tool->m_modeSwitch->setChecked(!tracking);
}

int Adalm2000PowerSupply_API::trackingPercent() const { return m_tool->m_ratioSlider->value(); }

void Adalm2000PowerSupply_API::setTrackingPercent(int percent)
{
	percent = qBound(ps::RATIO_MIN, percent, ps::RATIO_MAX);
	m_tool->m_ratioSlider->setValue(percent);
}

bool Adalm2000PowerSupply_API::run(int channel) { return setRailsEnabled(channel, true); }

bool Adalm2000PowerSupply_API::stop(int channel) { return setRailsEnabled(channel, false); }

bool Adalm2000PowerSupply_API::setRailsEnabled(int channel, bool on)
{
	if(channel == -1) {
		// Rail 1 follows rail 0 in tracking mode, so driving rail 0 is enough there.
		m_tool->m_enableBtn[0]->setChecked(on);
		if(!m_tool->m_tracking) {
			m_tool->m_enableBtn[1]->setChecked(on);
		}
		return true;
	}

	if(channel < 0 || channel > 1) {
		qWarning(CAT_ADALM2000POWERSUPPLY_API) << "channel must be 0, 1, or omitted for both; got" << channel;
		return false;
	}

	if(channel == 1 && m_tool->m_tracking) {
		qWarning(CAT_ADALM2000POWERSUPPLY_API) << "rail 1 is controlled by rail 0 in sync (tracking) mode";
		return false;
	}

	m_tool->m_enableBtn[channel]->setChecked(on);
	return true;
}

// Must stay out of a shared moc TU with the m2k* mocs: iioutil/command.h and
// core/command.h each declare scopy::Command behind independent include guards.
#include "moc_adalm2000powersupply_api.cpp"
