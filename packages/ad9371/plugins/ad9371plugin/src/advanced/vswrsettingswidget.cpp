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

#include "advanced/vswrsettingswidget.h"
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpacerItem>
#include <iio-widgets/iiowidget.h>
#include <QLoggingCategory>
#include <gui/widgets/menucollapsesection.h>
#include <gui/style.h>

Q_LOGGING_CATEGORY(CAT_AD9371_VSWR_SETTINGS, "AD9371_VSWR_SETTINGS")

using namespace scopy;
using namespace scopy::ad9371;

VswrSettingsWidget::VswrSettingsWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_VSWR_SETTINGS) << "No device provided to VSWR Settings";
		return;
	}

	setupUi();
}

VswrSettingsWidget::~VswrSettingsWidget() {}

void VswrSettingsWidget::setupUi()
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

	contentLayout->addWidget(createVswrSettingsSection(contentWidget));

	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);
}

QWidget *VswrSettingsWidget::createVswrSettingsSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		"VSWR Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	section->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// Signed 16-bit conversion lambdas (matches original SPINBUTTON_S16: (short)val cast)
	auto s16DataToUI = [](QString data) { return QString::number((qint16)data.toLongLong()); };
	auto s16UItoData = [](QString data) { return QString::number((quint16)(qint16)data.toInt()); };

	// VSWR Additional Delay Offset - RangeUi [-32768,32767,1] (signed 16-bit)
	auto vswrDelayOffsetWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,vswr-additional-delay-offset", "[-32768 1 32767]", "VSWR Additional Delay Offset", this);
	if(vswrDelayOffsetWidget) {
		vswrDelayOffsetWidget->setDataToUIConversion(s16DataToUI);
		vswrDelayOffsetWidget->setUItoDataConversion(s16UItoData);
		if(m_widgetGroup)
			m_widgetGroup->add(vswrDelayOffsetWidget);
		layout->addWidget(vswrDelayOffsetWidget);
		m_widgets.append(vswrDelayOffsetWidget);
		connect(this, &VswrSettingsWidget::readRequested, vswrDelayOffsetWidget, &IIOWidget::readAsync);
	}

	// VSWR Path Delay PN Seq Level - RangeUi [0,255,1]
	auto vswrPathDelayWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,vswr-path-delay-pn-seq-level", "[0 1 255]", "VSWR Path Delay PN Seq Level", this);
	if(vswrPathDelayWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(vswrPathDelayWidget);
		layout->addWidget(vswrPathDelayWidget);
		m_widgets.append(vswrPathDelayWidget);
		connect(this, &VswrSettingsWidget::readRequested, vswrPathDelayWidget, &IIOWidget::readAsync);
	}

	// VSWR TX1 Switch GPIO3v3 Pin - RangeUi [0,11,1]
	auto vswrTx1GpioPinWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,vswr-tx1-vswr-switch-gpio3p3-pin", "[0 1 11]", "VSWR TX1 Switch GPIO3v3 Pin", this);
	if(vswrTx1GpioPinWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(vswrTx1GpioPinWidget);
		layout->addWidget(vswrTx1GpioPinWidget);
		m_widgets.append(vswrTx1GpioPinWidget);
		connect(this, &VswrSettingsWidget::readRequested, vswrTx1GpioPinWidget, &IIOWidget::readAsync);
	}

	// VSWR TX2 Switch GPIO3v3 Pin - RangeUi [0,11,1]
	auto vswrTx2GpioPinWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,vswr-tx2-vswr-switch-gpio3p3-pin", "[0 1 11]", "VSWR TX2 Switch GPIO3v3 Pin", this);
	if(vswrTx2GpioPinWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(vswrTx2GpioPinWidget);
		layout->addWidget(vswrTx2GpioPinWidget);
		m_widgets.append(vswrTx2GpioPinWidget);
		connect(this, &VswrSettingsWidget::readRequested, vswrTx2GpioPinWidget, &IIOWidget::readAsync);
	}

	// VSWR TX1 Switch Polarity - CheckBoxUi
	auto vswrTx1PolarityWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,vswr-tx1-vswr-switch-polarity", "VSWR TX1 Switch Polarity", this);
	if(vswrTx1PolarityWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(vswrTx1PolarityWidget);
		layout->addWidget(vswrTx1PolarityWidget);
		m_widgets.append(vswrTx1PolarityWidget);
		connect(this, &VswrSettingsWidget::readRequested, vswrTx1PolarityWidget, &IIOWidget::readAsync);
	}

	// VSWR TX2 Switch Polarity - CheckBoxUi
	auto vswrTx2PolarityWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,vswr-tx2-vswr-switch-polarity", "VSWR TX2 Switch Polarity", this);
	if(vswrTx2PolarityWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(vswrTx2PolarityWidget);
		layout->addWidget(vswrTx2PolarityWidget);
		m_widgets.append(vswrTx2PolarityWidget);
		connect(this, &VswrSettingsWidget::readRequested, vswrTx2PolarityWidget, &IIOWidget::readAsync);
	}

	// VSWR TX1 Switch Delay (us) - RangeUi [0,255,1] (8-bit field)
	auto vswrTx1DelayWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,vswr-tx1-vswr-switch-delay_us", "[0 1 255]", "VSWR TX1 Switch Delay (us)", this);
	if(vswrTx1DelayWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(vswrTx1DelayWidget);
		layout->addWidget(vswrTx1DelayWidget);
		m_widgets.append(vswrTx1DelayWidget);
		connect(this, &VswrSettingsWidget::readRequested, vswrTx1DelayWidget, &IIOWidget::readAsync);
	}

	// VSWR TX2 Switch Delay (us) - RangeUi [0,255,1] (8-bit field)
	auto vswrTx2DelayWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,vswr-tx2-vswr-switch-delay_us", "[0 1 255]", "VSWR TX2 Switch Delay (us)", this);
	if(vswrTx2DelayWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(vswrTx2DelayWidget);
		layout->addWidget(vswrTx2DelayWidget);
		m_widgets.append(vswrTx2DelayWidget);
		connect(this, &VswrSettingsWidget::readRequested, vswrTx2DelayWidget, &IIOWidget::readAsync);
	}

	return section;
}
