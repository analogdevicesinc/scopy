/*
 * Copyright (c) 2025 Analog Devices Inc.
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
 *
 */

#include "fmcomms5/fmcomms5tab.h"

#include <style.h>
#include <iiowidgetbuilder.h>
#include <iiowidgetutils.h>
#include <QLoggingCategory>

#include <fmcomms5/fmcomms5calibration.h>

Q_LOGGING_CATEGORY(CAT_FMCOMMS5_TAB, "FMCOMMS5_TAB")

using namespace scopy;
using namespace ad936x;

Fmcomms5Tab::Fmcomms5Tab(iio_context *ctx, QWidget *parent)
	: m_ctx(ctx)
	, QWidget{parent}
{

	Style::setBackgroundColor(this, json::theme::background_primary);

	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_layout);

	QWidget *widget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(10);

	m_layout->addWidget(widget);

	Style::setStyle(widget, style::properties::widget::border_interactive);

	QLabel *title = new QLabel("CAL Switch Control", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	Fmcomms5Calibration *calibration = new Fmcomms5Calibration(ctx, this);

	QComboBox *calSwitchControl = new QComboBox(widget);

	calSwitchControl->addItem("DISABLE");
	calSwitchControl->addItem("TX1B_B->RX1C_B");
	calSwitchControl->addItem("TX1B_A->RX1C_B");
	calSwitchControl->addItem("TX1B_B->RX1C_A");
	calSwitchControl->addItem("TX1B_A->RX1C_A");

	layout->addWidget(calSwitchControl);

	connect(calSwitchControl, QOverload<const int>::of(&QComboBox::currentIndexChanged), this,
		[=](int idx) { calibration->callSwitchPortsEnableCb(idx); });

	m_calibrateBtn = new QPushButton("Calibrate", this);
	Style::setStyle(m_calibrateBtn, style::properties::button::basicButton);

	m_resetCalibrationBtn = new QPushButton("Reset Calibration", this);
	Style::setStyle(m_resetCalibrationBtn, style::properties::button::basicButton);
	connect(m_resetCalibrationBtn, &QPushButton::clicked, calibration, &Fmcomms5Calibration::resetCalibration);

	QHBoxLayout *calibBtnLayout = new QHBoxLayout();
	calibBtnLayout->setSpacing(10);

	calibBtnLayout->addWidget(m_calibrateBtn);
	calibBtnLayout->addWidget(m_resetCalibrationBtn);
	layout->addLayout(calibBtnLayout);

	m_calibProgressBar = new QProgressBar(this);
	m_calibProgressBar->setRange(0, 100);
	m_calibProgressBar->setValue(0);

	connect(calibration, &Fmcomms5Calibration::updateCalibrationProgress, m_calibProgressBar,
		&QProgressBar::setValue);

	layout->addWidget(m_calibProgressBar);

	iio_device *mainDevice = iio_context_find_device(m_ctx, "ad9361-phy");

	if(!mainDevice) {
		qWarning(CAT_FMCOMMS5_TAB) << "No ad9361-phy device found in context!";
		return;
	}

	// TX Phase
	IIOWidget *txPhase = IIOWidgetBuilder(widget)
				     .device(mainDevice)
				     .attribute("calibration_switch_control")
				     .uiStrategy(IIOWidgetBuilder::RangeUi)
				     .optionsValues("[0 0.1 360]")
				     .title("TX Phase")
				     .buildSingle();
	layout->addWidget(txPhase);

	connect(m_calibrateBtn, &QPushButton::clicked, this, [=]() {
		m_calibrateBtn->setEnabled(false);
		calibration->calibrate();
		m_calibrateBtn->setEnabled(true);
		txPhase->readAsync();
	});

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
}

Fmcomms5Tab::~Fmcomms5Tab() {}
