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
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpacerItem>
#include <QGroupBox>
#include <iio-widgets/iiowidget.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_AD9371_CALIBRATION, "AD9371_CALIBRATION")

using namespace scopy;
using namespace scopy::ad9371;

CalibrationWidget::CalibrationWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
	, m_txQecCal(nullptr)
	, m_txLoLeakageCal(nullptr)
	, m_txLoLeakageExtCal(nullptr)
	, m_txBbFilterCal(nullptr)
{
	if(!m_device) {
		qWarning(CAT_AD9371_CALIBRATION) << "No device provided to Calibrations";
		return;
	}

	setupUi();
	readCalibrationMaskFromDevice();

	Style::setStyle(this, style::properties::widget::border_interactive);
}

CalibrationWidget::~CalibrationWidget() {}

void CalibrationWidget::setupUi()
{
	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setWidgetResizable(true);

	QWidget *contentWidget = new QWidget();
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	MenuSectionCollapseWidget *calibrationSection = new MenuSectionCollapseWidget(
		"Calibrations", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, contentWidget);

	calibrationSection->contentLayout()->addWidget(createCalibrationMaskGroup(calibrationSection));
	connect(this, &CalibrationWidget::readRequested, this, &CalibrationWidget::readCalibrationMaskFromDevice);

	contentLayout->addWidget(calibrationSection);
	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_AD9371_CALIBRATION) << "Calibration widget created with 7 calibration mask controls";
}

QWidget *CalibrationWidget::createCalibrationMaskGroup(QWidget *parent)
{
	QGroupBox *widget = new QGroupBox("INIT Calibrations", parent);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 15, 10, 10);
	layout->setSpacing(10);

	// Bit 14: RX QEC
	m_txQecCal = new MenuOnOffSwitch("RX QEC", widget);
	m_txQecCal->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	layout->addWidget(m_txQecCal);

	// Bit 10: TX QEC
	m_txLoLeakageCal = new MenuOnOffSwitch("TX QEC", widget);
	m_txLoLeakageCal->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	layout->addWidget(m_txLoLeakageCal);

	// Bit 8: TX LOL
	m_txBbFilterCal = new MenuOnOffSwitch("TX LOL", widget);
	m_txBbFilterCal->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	layout->addWidget(m_txBbFilterCal);

	// Bit 9: External TX LOL
	m_txLoLeakageExtCal = new MenuOnOffSwitch("External TX LOL", widget);
	m_txLoLeakageExtCal->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	layout->addWidget(m_txLoLeakageExtCal);

	// Connect all checkboxes to shared callback
	connect(m_txQecCal->onOffswitch(), &QAbstractButton::toggled, this,
		&CalibrationWidget::onCalibrationMaskChanged);
	connect(m_txLoLeakageCal->onOffswitch(), &QAbstractButton::toggled, this,
		&CalibrationWidget::onCalibrationMaskChanged);
	connect(m_txLoLeakageExtCal->onOffswitch(), &QAbstractButton::toggled, this,
		&CalibrationWidget::onCalibrationMaskChanged);
	connect(m_txBbFilterCal->onOffswitch(), &QAbstractButton::toggled, this,
		&CalibrationWidget::onCalibrationMaskChanged);

	layout->addStretch();

	return widget;
}

void CalibrationWidget::onCalibrationMaskChanged() { writeCalibrationMaskToDevice(); }

void CalibrationWidget::readCalibrationMaskFromDevice()
{
	if(!m_device || !m_txQecCal || !m_txLoLeakageCal || !m_txLoLeakageExtCal || !m_txBbFilterCal) {
		return;
	}

	long long mask = 0;
	int ret = iio_device_debug_attr_read_longlong(m_device, "adi,default-initial-calibrations-mask", &mask);
	if(ret < 0) {
		qDebug(CAT_AD9371_CALIBRATION) << "Failed to read calibration mask, using defaults (all disabled)";
		mask = 0;
	}

	qDebug(CAT_AD9371_CALIBRATION) << "Read calibration mask from device:" << QString("0x%1").arg(mask, 0, 16);

	// Block signals to prevent triggering writes during read
	m_txQecCal->blockSignals(true);
	m_txLoLeakageCal->blockSignals(true);
	m_txLoLeakageExtCal->blockSignals(true);
	m_txBbFilterCal->blockSignals(true);

	m_txQecCal->onOffswitch()->setChecked((mask & (1LL << 14)) != 0);	  // Bit 14
	m_txLoLeakageCal->onOffswitch()->setChecked((mask & (1LL << 10)) != 0);	  // Bit 10
	m_txLoLeakageExtCal->onOffswitch()->setChecked((mask & (1LL << 9)) != 0); // Bit 9
	m_txBbFilterCal->onOffswitch()->setChecked((mask & (1LL << 8)) != 0);	  // Bit 8

	m_txQecCal->blockSignals(false);
	m_txLoLeakageCal->blockSignals(false);
	m_txLoLeakageExtCal->blockSignals(false);
	m_txBbFilterCal->blockSignals(false);
}

void CalibrationWidget::writeCalibrationMaskToDevice()
{
	if(!m_device || !m_txQecCal || !m_txLoLeakageCal || !m_txLoLeakageExtCal || !m_txBbFilterCal) {
		return;
	}

	// Read-modify-write: preserve bits not managed by this UI
	long long mask = 0;
	int ret = iio_device_debug_attr_read_longlong(m_device, "adi,default-initial-calibrations-mask", &mask);
	if(ret < 0) {
		qWarning(CAT_AD9371_CALIBRATION) << "Failed to read calibration mask before write, error:" << ret;
		return;
	}

	// Clear the 4 managed bits (8, 9, 10, 14)
	mask &= ~((1LL << 8) | (1LL << 9) | (1LL << 10) | (1LL << 14));

	if(m_txBbFilterCal->onOffswitch()->isChecked())
		mask |= (1LL << 8); // Bit 8: TX BB Filter
	if(m_txLoLeakageExtCal->onOffswitch()->isChecked())
		mask |= (1LL << 9); // Bit 9: TX LO Leakage External
	if(m_txLoLeakageCal->onOffswitch()->isChecked())
		mask |= (1LL << 10); // Bit 10: TX LO Leakage
	if(m_txQecCal->onOffswitch()->isChecked())
		mask |= (1LL << 14); // Bit 14: TX QEC

	qDebug(CAT_AD9371_CALIBRATION) << "Writing calibration mask to device:" << QString("0x%1").arg(mask, 0, 16);

	ret = iio_device_debug_attr_write_longlong(m_device, "adi,default-initial-calibrations-mask", mask);
	if(ret < 0) {
		qWarning(CAT_AD9371_CALIBRATION) << "Failed to write calibration mask, error:" << ret;
	}
}
