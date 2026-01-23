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

#include "advanced/armgpiowidget.h"
#include "adrv9009widgetfactory.h"
#include <gui/widgets/menucollapsesection.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QScrollArea>
#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_ARMGPIO, "ArmGpio")

using namespace scopy;
using namespace scopy::adrv9009;

ArmGpioWidget::ArmGpioWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
{
	if(!m_device) {
		qWarning(CAT_ARMGPIO) << "No device provided to ARM GPIO widget";
		return;
	}

	setupUi();
}

ArmGpioWidget::~ArmGpioWidget() {}

void ArmGpioWidget::setupUi()
{
	// Create main section with collapse functionality
	m_section = new MenuSectionCollapseWidget("ARM GPIO", MenuCollapseSection::MHCW_ARROW,
						  MenuCollapseSection::MHW_BASEWIDGET, this);

	// Main layout for this widget
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(m_section);

	// Add spacer to push MenuSectionCollapseWidget to top
	mainLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Create scroll area for content
	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	// Create content widget for scroll area
	QWidget *contentWidget = new QWidget();

	Style::setBackgroundColor(contentWidget, json::theme::background_primary);
	Style::setStyle(contentWidget, style::properties::widget::border_interactive);

	QGridLayout *contentLayout = new QGridLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(5);

	// ORX1 TX SEL0

	QLabel *label = new QLabel("ORX1 TX SEL0");
	Style::setStyle(label, style::properties::label::menuMedium);
	contentLayout->addWidget(label, 0, 0);

	// Enable widget
	auto orx1Sel0EnableWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,arm-gpio-config-orx1-tx-sel0-pin-enable", "Enable", contentWidget);
	if(orx1Sel0EnableWidget) {
		contentLayout->addWidget(orx1Sel0EnableWidget, 0, 1);
		orx1Sel0EnableWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &ArmGpioWidget::readRequested, orx1Sel0EnableWidget, &IIOWidget::readAsync);
	}

	// GPIO Pin Sel widget
	auto orx1Sel0PinWidget =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,arm-gpio-config-orx1-tx-sel0-pin-gpio-pin-sel",
							 "[0 1 15]", "GPIO Pin Sel", contentWidget);
	if(orx1Sel0PinWidget) {
		contentLayout->addWidget(orx1Sel0PinWidget, 0, 2);
		connect(this, &ArmGpioWidget::readRequested, orx1Sel0PinWidget, &IIOWidget::readAsync);
	}

	// Polarity widget
	auto orx1Sel0PolarityWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,arm-gpio-config-orx1-tx-sel0-pin-polarity", "Polarity", contentWidget);
	if(orx1Sel0PolarityWidget) {
		contentLayout->addWidget(orx1Sel0PolarityWidget, 0, 3);
		orx1Sel0PolarityWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &ArmGpioWidget::readRequested, orx1Sel0PolarityWidget, &IIOWidget::readAsync);
	}

	// ORX1 TX SEL1

	QLabel *label2 = new QLabel("ORX1 TX SEL1");
	Style::setStyle(label2, style::properties::label::menuMedium);
	contentLayout->addWidget(label2, 1, 0);

	// Enable widget
	auto orx1Sel1EnableWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,arm-gpio-config-orx2-tx-sel1-pin-enable", "Enable", contentWidget);
	if(orx1Sel1EnableWidget) {
		contentLayout->addWidget(orx1Sel1EnableWidget, 1, 1);
		orx1Sel1EnableWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &ArmGpioWidget::readRequested, orx1Sel1EnableWidget, &IIOWidget::readAsync);
	}

	// GPIO Pin Sel widget
	auto orx1Sel1PinWidget =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,arm-gpio-config-orx1-tx-sel1-pin-gpio-pin-sel",
							 "[0 1 15]", "GPIO Pin Sel", contentWidget);
	if(orx1Sel1PinWidget) {
		contentLayout->addWidget(orx1Sel1PinWidget, 1, 2);
		connect(this, &ArmGpioWidget::readRequested, orx1Sel1PinWidget, &IIOWidget::readAsync);
	}

	// Polarity widget
	auto orx1Sel1PolarityWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,arm-gpio-config-orx1-tx-sel1-pin-polarity", "Polarity", contentWidget);
	if(orx1Sel1PolarityWidget) {
		contentLayout->addWidget(orx1Sel1PolarityWidget, 1, 3);
		orx1Sel1PolarityWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &ArmGpioWidget::readRequested, orx1Sel1PolarityWidget, &IIOWidget::readAsync);
	}

	// ORX2 TX SEL0

	QLabel *label3 = new QLabel("ORX2 TX SEL0");
	Style::setStyle(label3, style::properties::label::menuMedium);
	contentLayout->addWidget(label3, 2, 0);

	// Enable widget
	auto orx2Sel0EnableWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,arm-gpio-config-orx2-tx-sel0-pin-enable", "Enable", contentWidget);
	if(orx2Sel0EnableWidget) {
		contentLayout->addWidget(orx2Sel0EnableWidget, 2, 1);
		orx2Sel0EnableWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &ArmGpioWidget::readRequested, orx2Sel0EnableWidget, &IIOWidget::readAsync);
	}

	// GPIO Pin Sel widget
	auto orx2Sel0PinWidget =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,arm-gpio-config-orx2-tx-sel0-pin-gpio-pin-sel",
							 "[0 1 15]", "GPIO Pin Sel", contentWidget);
	if(orx2Sel0PinWidget) {
		contentLayout->addWidget(orx2Sel0PinWidget, 2, 2);
		connect(this, &ArmGpioWidget::readRequested, orx2Sel0PinWidget, &IIOWidget::readAsync);
	}

	// Polarity widget
	auto orx2Sel0PolarityWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,arm-gpio-config-orx2-tx-sel0-pin-polarity", "Polarity", contentWidget);
	if(orx2Sel0PolarityWidget) {
		contentLayout->addWidget(orx2Sel0PolarityWidget, 2, 3);
		orx2Sel0PolarityWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &ArmGpioWidget::readRequested, orx2Sel0PolarityWidget, &IIOWidget::readAsync);
	}

	// ORX2 TX SEL1

	QLabel *label4 = new QLabel("ORX2 TX SEL1");
	Style::setStyle(label4, style::properties::label::menuMedium);
	contentLayout->addWidget(label4, 3, 0);

	// Enable widget
	auto orx2Sel1EnableWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,arm-gpio-config-orx2-tx-sel1-pin-enable", "Enable", contentWidget);
	if(orx2Sel1EnableWidget) {
		contentLayout->addWidget(orx2Sel1EnableWidget, 3, 1);
		orx2Sel1EnableWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &ArmGpioWidget::readRequested, orx2Sel1EnableWidget, &IIOWidget::readAsync);
	}

	// GPIO Pin Sel widget
	auto orx2Sel1PinWidget =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,arm-gpio-config-orx2-tx-sel1-pin-gpio-pin-sel",
							 "[0 1 15]", "GPIO Pin Sel", contentWidget);
	if(orx2Sel1PinWidget) {
		contentLayout->addWidget(orx2Sel1PinWidget, 3, 2);
		connect(this, &ArmGpioWidget::readRequested, orx2Sel1PinWidget, &IIOWidget::readAsync);
	}

	// Polarity widget
	auto orx2Sel1PolarityWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,arm-gpio-config-orx2-tx-sel1-pin-polarity", "Polarity", contentWidget);
	if(orx2Sel1PolarityWidget) {
		contentLayout->addWidget(orx2Sel1PolarityWidget, 3, 3);
		orx2Sel1PolarityWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &ArmGpioWidget::readRequested, orx2Sel1PolarityWidget, &IIOWidget::readAsync);
	}

	// ENABLE TRACKING CALS

	QLabel *label5 = new QLabel("ENABLE TRACKING CALS CALIBRATION");
	Style::setStyle(label5, style::properties::label::menuMedium);
	contentLayout->addWidget(label5, 4, 0);

	// Enable widget
	auto trackingEnableWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,arm-gpio-config-en-tx-tracking-cals-enable", "Enable", contentWidget);
	if(trackingEnableWidget) {
		contentLayout->addWidget(trackingEnableWidget, 4, 1);
		trackingEnableWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &ArmGpioWidget::readRequested, trackingEnableWidget, &IIOWidget::readAsync);
	}

	// GPIO Pin Sel widget
	auto trackingPinWidget = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,arm-gpio-config-en-tx-tracking-cals-gpio-pin-sel", "[0 1 15]", "GPIO Pin Sel",
		contentWidget);
	if(trackingPinWidget) {
		contentLayout->addWidget(trackingPinWidget, 4, 2);
		connect(this, &ArmGpioWidget::readRequested, trackingPinWidget, &IIOWidget::readAsync);
	}

	// Polarity widget
	auto trackingPolarityWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,arm-gpio-config-en-tx-tracking-cals-polarity", "Polarity", contentWidget);
	if(trackingPolarityWidget) {
		contentLayout->addWidget(trackingPolarityWidget, 4, 3);
		trackingPolarityWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &ArmGpioWidget::readRequested, trackingPolarityWidget, &IIOWidget::readAsync);
	}

	// Set up scroll area
	scrollArea->setWidget(contentWidget);
	m_section->contentLayout()->addWidget(scrollArea);

	qDebug(CAT_ARMGPIO) << "ARM GPIO widget created with 15 attributes";
}

MenuSectionCollapseWidget *ArmGpioWidget::section() const { return m_section; }
