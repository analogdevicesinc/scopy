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
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpacerItem>
#include <QGroupBox>
#include <iio-widgets/iiowidget.h>
#include <QLoggingCategory>
#include <style.h>
#include <gui/widgets/menucollapsesection.h>

Q_LOGGING_CATEGORY(CAT_AD9371_ARM_GPIO, "AD9371_ARM_GPIO")

using namespace scopy;
using namespace scopy::ad9371;

ArmGpioWidget::ArmGpioWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_ARM_GPIO) << "No device provided to ARM GPIO";
		return;
	}

	setupUi();
}

ArmGpioWidget::~ArmGpioWidget() {}

void ArmGpioWidget::setupUi()
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

	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		"ARM GPIO", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, contentWidget);

	// ARM GPIO Config section (8 standard debug attrs)
	section->contentLayout()->addWidget(createGpioConfigSection(section));

	// ARM GPIO Enable ACK section (8 range widgets + 8 bit-4 checkboxes)
	section->contentLayout()->addWidget(createEnableAckSection(section));

	// Connect refresh signal for custom bit-4 checkboxes
	connect(this, &ArmGpioWidget::readRequested, this, &ArmGpioWidget::readEnableAckFromDevice);

	contentLayout->addWidget(section);
	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_AD9371_ARM_GPIO) << "ARM GPIO widget created with 24 attributes";
}

QWidget *ArmGpioWidget::createGpioConfigSection(QWidget *parent)
{
	QGroupBox *group = new QGroupBox("Mode", parent);
	Style::setStyle(group, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(group);
	layout->setContentsMargins(10, 15, 10, 10);
	layout->setSpacing(5);

	// 4 checkboxes
	struct
	{
		const char *attr;
		const char *title;
	} checkboxAttrs[] = {
		{"adi,arm-gpio-use-rx2-enable-pin", "USE RX2 ENABLE PIN"},
		{"adi,arm-gpio-use-tx2-enable-pin", "USE TX2 ENABLE PIN"},
		{"adi,arm-gpio-tx-rx-pin-mode", "TX RX PIN MODE"},
		{"adi,arm-gpio-orx-pin-mode", "ORX PIN MODE"},
	};

	for(const auto &a : checkboxAttrs) {
		auto w = Ad9371WidgetFactory::createDebugCheckboxWidget(m_device, a.attr, a.title, group);
		if(w) {
			if(m_widgetGroup) m_widgetGroup->add(w);
			layout->addWidget(w);
			m_widgets.append(w);
			connect(this, &ArmGpioWidget::readRequested, w, &IIOWidget::readAsync);
		}
	}

	// 4 range widgets [0,15,1]
	struct
	{
		const char *attr;
		const char *title;
	} rangeAttrs[] = {
		{"adi,arm-gpio-orx-trigger-pin", "ORX TRIGGER PIN"},
		{"adi,arm-gpio-orx-mode2-pin", "ORX MODE2 PIN"},
		{"adi,arm-gpio-orx-mode1-pin", "ORX MODE1 PIN"},
		{"adi,arm-gpio-orx-mode0-pin", "ORX MODE0 PIN"},
	};

	for(const auto &a : rangeAttrs) {
		auto w = Ad9371WidgetFactory::createDebugRangeWidget(m_device, a.attr, "[0 1 15]", a.title, group);
		if(w) {
			if(m_widgetGroup) m_widgetGroup->add(w);
			layout->addWidget(w);
			m_widgets.append(w);
			connect(this, &ArmGpioWidget::readRequested, w, &IIOWidget::readAsync);
		}
	}

	return group;
}

QWidget *ArmGpioWidget::createEnableAckSection(QWidget *parent)
{
	QGroupBox *group = new QGroupBox("ACK", parent);
	Style::setStyle(group, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(group);
	layout->setContentsMargins(10, 15, 10, 10);
	layout->setSpacing(5);

	// Each enable-ack attr has a lower 4-bit range widget and a bit-4 checkbox.
	// The range widget reads/writes the lower 4 bits via standard IIOWidget [0,15,1].
	// The bit-4 checkbox is a custom MenuOnOffSwitch using read-modify-write.

	struct
	{
		const char *attr;
		const char *rangeTitle;
		const char *bit4Title;
	} enableAckAttrs[] = {
		{"adi,arm-gpio-rx1-enable-ack", "RX1 ENABLE ACK", "ENABLE"},
		{"adi,arm-gpio-rx2-enable-ack", "RX2 ENABLE ACK", "ENABLE"},
		{"adi,arm-gpio-tx1-enable-ack", "TX1 ENABLE ACK", "ENABLE"},
		{"adi,arm-gpio-tx2-enable-ack", "TX2 ENABLE ACK", "ENABLE"},
		{"adi,arm-gpio-orx1-enable-ack", "ORX1 ENABLE ACK", "ENABLE"},
		{"adi,arm-gpio-orx2-enable-ack", "ORX2 ENABLE ACK", "ENABLE"},
		{"adi,arm-gpio-srx-enable-ack", "SRX ENABLE ACK", "ENABLE"},
		{"adi,arm-gpio-tx-obs-select", "TX OBS SELECT", "ENABLE"},
	};

	for(const auto &a : enableAckAttrs) {
		// Row container for range + bit4 checkbox
		QWidget *row = new QWidget(group);
		QHBoxLayout *rowLayout = new QHBoxLayout(row);
		rowLayout->setContentsMargins(0, 0, 0, 0);
		rowLayout->setSpacing(10);

		// Range widget for lower 4 bits [0,15,1]
		auto rangeWidget =
			Ad9371WidgetFactory::createDebugRangeWidget(m_device, a.attr, "[0 1 15]", a.rangeTitle, row);
		if(rangeWidget) {
			if(m_widgetGroup) m_widgetGroup->add(rangeWidget);
			rowLayout->addWidget(rangeWidget);
			m_widgets.append(rangeWidget);
			connect(this, &ArmGpioWidget::readRequested, rangeWidget, &IIOWidget::readAsync);
		}

		// Bit 4 checkbox (CHECKBOX_MASK pattern)
		MenuOnOffSwitch *bit4Switch = new MenuOnOffSwitch(a.bit4Title, row);
		rowLayout->addWidget(bit4Switch);
		m_enableAckBit4.append(bit4Switch);
		m_enableAckAttrNames.append(a.attr);

		connect(bit4Switch->onOffswitch(), &QCheckBox::toggled, this, &ArmGpioWidget::onEnableAckChanged);

		layout->addWidget(row);
	}

	return group;
}

void ArmGpioWidget::onEnableAckChanged() { writeEnableAckToDevice(); }

void ArmGpioWidget::readEnableAckFromDevice()
{
	if(!m_device) {
		return;
	}

	for(int i = 0; i < m_enableAckBit4.size() && i < m_enableAckAttrNames.size(); i++) {
		long long val = 0;
		int ret = iio_device_debug_attr_read_longlong(m_device, m_enableAckAttrNames[i].toStdString().c_str(),
							      &val);
		if(ret < 0) {
			qDebug(CAT_AD9371_ARM_GPIO)
				<< "Failed to read" << m_enableAckAttrNames[i] << "for bit 4, error:" << ret;
			continue;
		}

		// Read bit 4
		bool bit4Set = (val & (1 << 4)) != 0;

		m_enableAckBit4[i]->blockSignals(true);
		m_enableAckBit4[i]->onOffswitch()->setChecked(bit4Set);
		m_enableAckBit4[i]->blockSignals(false);
	}
}

void ArmGpioWidget::writeEnableAckToDevice()
{
	if(!m_device) {
		return;
	}

	for(int i = 0; i < m_enableAckBit4.size() && i < m_enableAckAttrNames.size(); i++) {
		// Read-modify-write: read current value, update bit 4, write back
		long long val = 0;
		int ret = iio_device_debug_attr_read_longlong(m_device, m_enableAckAttrNames[i].toStdString().c_str(),
							      &val);
		if(ret < 0) {
			qWarning(CAT_AD9371_ARM_GPIO)
				<< "Failed to read" << m_enableAckAttrNames[i] << "before write, error:" << ret;
			continue;
		}

		// Clear bit 4, then set it based on checkbox state
		val &= ~(1 << 4);
		if(m_enableAckBit4[i]->onOffswitch()->isChecked()) {
			val |= (1 << 4);
		}

		ret = iio_device_debug_attr_write_longlong(m_device, m_enableAckAttrNames[i].toStdString().c_str(),
							   val);
		if(ret < 0) {
			qWarning(CAT_AD9371_ARM_GPIO)
				<< "Failed to write" << m_enableAckAttrNames[i] << "error:" << ret;
		}
	}
}
