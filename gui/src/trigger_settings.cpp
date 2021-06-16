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

#include "scroll_filter.hpp"

#include "ui_trigger_settings.h"

#include <iio.h>

#include <QDebug>
#include <QTranslator>

#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/context.hpp>
#include <libm2k/contextbuilder.hpp>
#include <libm2k/m2k.hpp>
#include <libm2k/m2kexceptions.hpp>
#include <libm2k/m2khardwaretrigger.hpp>
#include <scopy/core/scopy_exception_handler.hpp>
#include <scopy/gui/spinbox_a.hpp>
#include <scopy/gui/trigger_settings.hpp>

using namespace scopy::gui;

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

TriggerSettings::TriggerSettings(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::TriggerSettings)
	, m_triggerIn(false)
	, m_digitalTriggerWasOn(false)
	, m_currentChannel(0)
	, m_temporarilyDisabled(false)
	, m_triggerRawDelay(0)
	, m_daisyChainCompensation(0)
	, m_adcRunning(false)
	, m_hasExternalTriggerOut(false)

{
	m_ui->setupUi(this);

	m_triggerLevel = new PositionSpinButton({{tr("μVolts"), 1e-6}, {tr("mVolts"), 1e-3}, {tr("Volts"), 1e0}},
						tr("Level"), 0.0, 0.0, true, false, this);

	m_triggerHysteresis = new PositionSpinButton({{tr("μVolts"), 1e-6}, {tr("mVolts"), 1e-3}, {tr("Volts"), 1e0}},
						     tr("Hysteresis"), 0.0, 0.0, true, false, this);

	m_ui->vLayoutControls->addWidget(m_triggerLevel);
	m_ui->vLayoutControls->addWidget(m_triggerHysteresis);

	m_triggerAutoMode = m_ui->btnTrigger->isChecked();

	// Default GUI settings
	m_ui->cmbBoxSource->setCurrentIndex(0);
	m_ui->btnInternEn->setChecked(true);
	m_ui->btnExternEn->setChecked(false);
	m_ui->cmbBoxCondition->setCurrentIndex(0);
	m_ui->cmbBoxExternCondition->setCurrentIndex(0);
	m_ui->cmbBoxAnalogExtern->setCurrentIndex(0);

	m_ui->lblDaisyChain->setVisible(false);
	m_ui->spinBoxDaisyChain->setVisible(false);

	m_triggerLevel->setValue(0);
	m_acCoupled = false;
	m_triggerHysteresis->setValue(50e-3);
	MouseWheelWidgetGuard* wheelEventGuard = new MouseWheelWidgetGuard(this);
	wheelEventGuard->installEventRecursively(this);

	m_ui->lblMixedSignal->setVisible(false);
	m_ui->btnLogicAnalyzerTriggers->setVisible(false);

	m_ui->widgetTriggerSettingsMenuHeader->setLabel("Trigger Settings");
	m_ui->widgetTriggerSettingsMenuHeader->setLineColor(new QColor("#4A64FF"));

	m_ui->widgetDigitalSubsSep->setLabel("DIGITAL");
	m_ui->widgetDigitalSubsSep->setButtonVisible(false);

	m_ui->widgetTriggerLogicSubsSep->setLabel("TRIGGER LOGIC");
	m_ui->widgetTriggerLogicSubsSep->setButtonVisible(false);

	m_ui->widgetInternalAnalogSubsSep->setLabel("INTERNAL (ANALOG)");
	m_ui->widgetInternalAnalogSubsSep->setButtonVisible(false);

	m_ui->widgetExternalTriggerOutSubsSep->setLabel("EXTERNAL TRIGGER OUT");
	m_ui->widgetExternalTriggerOutSubsSep->setButtonVisible(false);
}

TriggerSettings::~TriggerSettings() { delete m_ui; }
