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
 */

#include "advanced/calibrationwidget.h"
#include <gui/widgets/menucollapsesection.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QScrollArea>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QLoggingCategory>
#include <style.h>
#include <QThread>
#include <QTimer>

Q_LOGGING_CATEGORY(CAT_CALIBRATION, "CALIBRATION")

using namespace scopy;
using namespace scopy::adrv9009;

CalibrationWidget::CalibrationWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_txLoLeakageCal(nullptr)
	, m_txQecCal(nullptr)
	, m_txLolExternalCal(nullptr)
	, m_rxQecCal(nullptr)
	, m_rxPhaseCorrectionCal(nullptr)
	, m_fhmCal(nullptr)
{
	if(!m_device) {
		qWarning(CAT_CALIBRATION) << "No device provided to Calibration widget";
		return;
	}

	setupUi();
	// get initial values from device
	readCalibrationMaskFromDevice();

	Style::setStyle(this, style::properties::widget::border_interactive);
}

CalibrationWidget::~CalibrationWidget() {}

void CalibrationWidget::setupUi()
{
	// Main layout for this widget
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	// Create scroll area for all sections
	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	// Create content widget for scroll area
	QWidget *contentWidget = new QWidget();
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	// Add Calibration section
	MenuSectionCollapseWidget *calibrationSection = new MenuSectionCollapseWidget(
		"Calibrations", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, contentWidget);

	calibrationSection->contentLayout()->addWidget(createCalibrationMaskGroup(calibrationSection));
	connect(this, &CalibrationWidget::readRequested, this, &CalibrationWidget::readCalibrationMaskFromDevice);

	contentLayout->addWidget(calibrationSection);
	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_CALIBRATION) << "Calibration widget created with calibration mask controls (6 calibration types) "
				   "and Initialize button";
}

QWidget *CalibrationWidget::createCalibrationMaskGroup(QWidget *parent)
{
	// Calibration mask section with frame
	QGroupBox *widget = new QGroupBox("Initial Calibrations Mask", parent);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	mainLayout->setContentsMargins(10, 15, 10, 10);
	mainLayout->setSpacing(10);

	// Create calibration checkboxes
	m_txLoLeakageCal = new MenuOnOffSwitch("TX LO Leakage", widget);
	m_txQecCal = new MenuOnOffSwitch("TX QEC", widget);
	m_txLolExternalCal = new MenuOnOffSwitch("TX LOL External", widget);
	m_rxQecCal = new MenuOnOffSwitch("RX QEC", widget);
	m_rxPhaseCorrectionCal = new MenuOnOffSwitch("RX Phase Coorrection", widget);
	m_fhmCal = new MenuOnOffSwitch("FHM", widget);

	mainLayout->addWidget(m_rxQecCal);
	mainLayout->addWidget(m_txQecCal);
	mainLayout->addWidget(m_txLoLeakageCal);
	mainLayout->addWidget(m_txLolExternalCal);

	mainLayout->addWidget(m_rxPhaseCorrectionCal);
	mainLayout->addWidget(m_fhmCal);
	mainLayout->addStretch();

	// Connect all calibration checkboxes to shared callback (BistWidget pattern)
	connect(m_txLoLeakageCal->onOffswitch(), &QCheckBox::toggled, this,
		&CalibrationWidget::onCalibrationMaskChanged);
	connect(m_txQecCal->onOffswitch(), &QCheckBox::toggled, this, &CalibrationWidget::onCalibrationMaskChanged);
	connect(m_txLolExternalCal->onOffswitch(), &QCheckBox::toggled, this,
		&CalibrationWidget::onCalibrationMaskChanged);
	connect(m_rxQecCal->onOffswitch(), &QCheckBox::toggled, this, &CalibrationWidget::onCalibrationMaskChanged);
	connect(m_rxPhaseCorrectionCal->onOffswitch(), &QCheckBox::toggled, this,
		&CalibrationWidget::onCalibrationMaskChanged);
	connect(m_fhmCal->onOffswitch(), &QCheckBox::toggled, this, &CalibrationWidget::onCalibrationMaskChanged);

	return widget;
}

void CalibrationWidget::onCalibrationMaskChanged()
{
	// Called when any calibration checkbox changes - write immediately (BistWidget pattern)
	writeCalibrationMaskToDevice();
}

void CalibrationWidget::readCalibrationMaskFromDevice()
{
	if(!m_device || !m_txLoLeakageCal || !m_txQecCal || !m_txLolExternalCal || !m_rxQecCal ||
	   !m_rxPhaseCorrectionCal || !m_fhmCal) {
		return;
	}

	// Read calibration mask from device
	long long mask = 0;
	int ret = iio_device_debug_attr_read_longlong(m_device, "adi,default-initial-calibrations-mask", &mask);
	if(ret < 0) {
		// Set default values (all calibrations disabled) on read error
		qDebug(CAT_CALIBRATION) << "Failed to read calibration mask, using defaults (all disabled)";
		mask = 0;
	}

	qDebug(CAT_CALIBRATION) << "Read calibration mask from device:" << QString("0x%1").arg(mask, 0, 16);

	// Temporarily disconnect signals to prevent triggering onCalibrationMaskChanged
	m_txLoLeakageCal->blockSignals(true);
	m_txQecCal->blockSignals(true);
	m_txLolExternalCal->blockSignals(true);
	m_rxQecCal->blockSignals(true);
	m_rxPhaseCorrectionCal->blockSignals(true);
	m_fhmCal->blockSignals(true);

	// Update checkboxes based on bitmask (specific bits: 8, 9, 10, 14, 15, 23)
	m_txLoLeakageCal->onOffswitch()->setChecked((mask & (1LL << 8)) != 0);	      // Bit 8: TX LO Leakage
	m_txLolExternalCal->onOffswitch()->setChecked((mask & (1LL << 9)) != 0);      // Bit 9: TX LOL External
	m_txQecCal->onOffswitch()->setChecked((mask & (1LL << 10)) != 0);	      // Bit 10: TX QEC
	m_rxQecCal->onOffswitch()->setChecked((mask & (1LL << 14)) != 0);	      // Bit 14: RX QEC
	m_rxPhaseCorrectionCal->onOffswitch()->setChecked((mask & (1LL << 15)) != 0); // Bit 15: RX Phase correction
	m_fhmCal->onOffswitch()->setChecked((mask & (1LL << 23)) != 0);		      // Bit 23: FHM

	// Re-enable signals
	m_txLoLeakageCal->blockSignals(false);
	m_txQecCal->blockSignals(false);
	m_txLolExternalCal->blockSignals(false);
	m_rxQecCal->blockSignals(false);
	m_rxPhaseCorrectionCal->blockSignals(false);
	m_fhmCal->blockSignals(false);
}

void CalibrationWidget::writeCalibrationMaskToDevice()
{
	if(!m_device || !m_txLoLeakageCal || !m_txQecCal || !m_txLolExternalCal || !m_rxQecCal ||
	   !m_rxPhaseCorrectionCal || !m_fhmCal) {
		return;
	}

	// Build calibration mask from UI checkboxes (specific bits: 8, 9, 10, 14, 15, 23)
	long long mask = 0;

	if(m_txLoLeakageCal->onOffswitch()->isChecked())
		mask |= (1LL << 8); // Bit 8: TX LO Leakage
	if(m_txLolExternalCal->onOffswitch()->isChecked())
		mask |= (1LL << 9); // Bit 9: TX LOL External
	if(m_txQecCal->onOffswitch()->isChecked())
		mask |= (1LL << 10); // Bit 10: TX QEC
	if(m_rxQecCal->onOffswitch()->isChecked())
		mask |= (1LL << 14); // Bit 14: RX QEC
	if(m_rxPhaseCorrectionCal->onOffswitch()->isChecked())
		mask |= (1LL << 15); // Bit 15: RX Phase correction
	if(m_fhmCal->onOffswitch()->isChecked())
		mask |= (1LL << 23); // Bit 23: FHM

	qDebug(CAT_CALIBRATION) << "Writing calibration mask to device:" << QString("0x%1").arg(mask, 0, 16);

	// Write calibration mask to device
	int ret = iio_device_debug_attr_write_longlong(m_device, "adi,default-initial-calibrations-mask", mask);
	if(ret < 0) {
		qWarning(CAT_CALIBRATION) << "Failed to write calibration mask, error:" << ret;
	}
}
