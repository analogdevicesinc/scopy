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

#include "advanced/clgcsettingswidget.h"
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

Q_LOGGING_CATEGORY(CAT_AD9371_CLGC_SETTINGS, "AD9371_CLGC_SETTINGS")

using namespace scopy;
using namespace scopy::ad9371;

ClgcSettingsWidget::ClgcSettingsWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_CLGC_SETTINGS) << "No device provided to CLGC Settings";
		return;
	}

	setupUi();
}

ClgcSettingsWidget::~ClgcSettingsWidget() {}

void ClgcSettingsWidget::setupUi()
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

	contentLayout->addWidget(createClgcSettingsSection(contentWidget));

	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);
}

QWidget *ClgcSettingsWidget::createClgcSettingsSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		"CLGC Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

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

	// CLGC TX1 Desired Gain - RangeUi [-32768,32767,1] (signed 16-bit)
	auto clgcTx1DesiredGainWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,clgc-tx1-desired-gain", "[-32768 1 32767]", "CLGC TX1 Desired Gain", this);
	if(clgcTx1DesiredGainWidget) {
		clgcTx1DesiredGainWidget->setDataToUIConversion(s16DataToUI);
		clgcTx1DesiredGainWidget->setUItoDataConversion(s16UItoData);
		if(m_widgetGroup)
			m_widgetGroup->add(clgcTx1DesiredGainWidget);
		layout->addWidget(clgcTx1DesiredGainWidget);
		m_widgets.append(clgcTx1DesiredGainWidget);
		connect(this, &ClgcSettingsWidget::readRequested, clgcTx1DesiredGainWidget, &IIOWidget::readAsync);
	}

	// CLGC TX2 Desired Gain - RangeUi [-32768,32767,1] (signed 16-bit)
	auto clgcTx2DesiredGainWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,clgc-tx2-desired-gain", "[-32768 1 32767]", "CLGC TX2 Desired Gain", this);
	if(clgcTx2DesiredGainWidget) {
		clgcTx2DesiredGainWidget->setDataToUIConversion(s16DataToUI);
		clgcTx2DesiredGainWidget->setUItoDataConversion(s16UItoData);
		if(m_widgetGroup)
			m_widgetGroup->add(clgcTx2DesiredGainWidget);
		layout->addWidget(clgcTx2DesiredGainWidget);
		m_widgets.append(clgcTx2DesiredGainWidget);
		connect(this, &ClgcSettingsWidget::readRequested, clgcTx2DesiredGainWidget, &IIOWidget::readAsync);
	}

	// CLGC TX1 Atten Limit - RangeUi [0,40000,1]
	auto clgcTx1AttenLimitWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,clgc-tx1-atten-limit", "[0 1 40000]", "CLGC TX1 Atten Limit", this);
	if(clgcTx1AttenLimitWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(clgcTx1AttenLimitWidget);
		layout->addWidget(clgcTx1AttenLimitWidget);
		m_widgets.append(clgcTx1AttenLimitWidget);
		connect(this, &ClgcSettingsWidget::readRequested, clgcTx1AttenLimitWidget, &IIOWidget::readAsync);
	}

	// CLGC TX2 Atten Limit - RangeUi [0,40000,1]
	auto clgcTx2AttenLimitWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,clgc-tx2-atten-limit", "[0 1 40000]", "CLGC TX2 Atten Limit", this);
	if(clgcTx2AttenLimitWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(clgcTx2AttenLimitWidget);
		layout->addWidget(clgcTx2AttenLimitWidget);
		m_widgets.append(clgcTx2AttenLimitWidget);
		connect(this, &ClgcSettingsWidget::readRequested, clgcTx2AttenLimitWidget, &IIOWidget::readAsync);
	}

	// CLGC TX1 Control Ratio - RangeUi [1,6,1]
	auto clgcTx1ControlRatioWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,clgc-tx1-control-ratio", "[1 1 6]", "CLGC TX1 Control Ratio", this);
	if(clgcTx1ControlRatioWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(clgcTx1ControlRatioWidget);
		layout->addWidget(clgcTx1ControlRatioWidget);
		m_widgets.append(clgcTx1ControlRatioWidget);
		connect(this, &ClgcSettingsWidget::readRequested, clgcTx1ControlRatioWidget, &IIOWidget::readAsync);
	}

	// CLGC TX2 Control Ratio - RangeUi [1,6,1]
	auto clgcTx2ControlRatioWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,clgc-tx2-control-ratio", "[1 1 6]", "CLGC TX2 Control Ratio", this);
	if(clgcTx2ControlRatioWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(clgcTx2ControlRatioWidget);
		layout->addWidget(clgcTx2ControlRatioWidget);
		m_widgets.append(clgcTx2ControlRatioWidget);
		connect(this, &ClgcSettingsWidget::readRequested, clgcTx2ControlRatioWidget, &IIOWidget::readAsync);
	}

	// CLGC Allow TX1 Atten Updates - CheckBoxUi
	auto clgcAllowTx1AttenWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,clgc-allow-tx1-atten-updates", "CLGC Allow TX1 Atten Updates", this);
	if(clgcAllowTx1AttenWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(clgcAllowTx1AttenWidget);
		layout->addWidget(clgcAllowTx1AttenWidget);
		m_widgets.append(clgcAllowTx1AttenWidget);
		connect(this, &ClgcSettingsWidget::readRequested, clgcAllowTx1AttenWidget, &IIOWidget::readAsync);
	}

	// CLGC Allow TX2 Atten Updates - CheckBoxUi
	auto clgcAllowTx2AttenWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,clgc-allow-tx2-atten-updates", "CLGC Allow TX2 Atten Updates", this);
	if(clgcAllowTx2AttenWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(clgcAllowTx2AttenWidget);
		layout->addWidget(clgcAllowTx2AttenWidget);
		m_widgets.append(clgcAllowTx2AttenWidget);
		connect(this, &ClgcSettingsWidget::readRequested, clgcAllowTx2AttenWidget, &IIOWidget::readAsync);
	}

	// CLGC Additional Delay Offset - RangeUi [-32768,32767,1] (signed 16-bit)
	auto clgcDelayOffsetWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,clgc-additional-delay-offset", "[-32768 1 32767]", "CLGC Additional Delay Offset", this);
	if(clgcDelayOffsetWidget) {
		clgcDelayOffsetWidget->setDataToUIConversion(s16DataToUI);
		clgcDelayOffsetWidget->setUItoDataConversion(s16UItoData);
		if(m_widgetGroup)
			m_widgetGroup->add(clgcDelayOffsetWidget);
		layout->addWidget(clgcDelayOffsetWidget);
		m_widgets.append(clgcDelayOffsetWidget);
		connect(this, &ClgcSettingsWidget::readRequested, clgcDelayOffsetWidget, &IIOWidget::readAsync);
	}

	// CLGC Path Delay PN Seq Level - RangeUi [0,255,1]
	auto clgcPathDelayWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,clgc-path-delay-pn-seq-level", "[0 1 255]", "CLGC Path Delay PN Seq Level", this);
	if(clgcPathDelayWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(clgcPathDelayWidget);
		layout->addWidget(clgcPathDelayWidget);
		m_widgets.append(clgcPathDelayWidget);
		connect(this, &ClgcSettingsWidget::readRequested, clgcPathDelayWidget, &IIOWidget::readAsync);
	}

	// CLGC TX1 Rel Threshold - RangeUi [0,255,1]
	auto clgcTx1RelThreshWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,clgc-tx1-rel-threshold", "[0 1 255]", "CLGC TX1 Rel Threshold", this);
	if(clgcTx1RelThreshWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(clgcTx1RelThreshWidget);
		layout->addWidget(clgcTx1RelThreshWidget);
		m_widgets.append(clgcTx1RelThreshWidget);
		connect(this, &ClgcSettingsWidget::readRequested, clgcTx1RelThreshWidget, &IIOWidget::readAsync);
	}

	// CLGC TX2 Rel Threshold - RangeUi [0,255,1]
	auto clgcTx2RelThreshWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,clgc-tx2-rel-threshold", "[0 1 255]", "CLGC TX2 Rel Threshold", this);
	if(clgcTx2RelThreshWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(clgcTx2RelThreshWidget);
		layout->addWidget(clgcTx2RelThreshWidget);
		m_widgets.append(clgcTx2RelThreshWidget);
		connect(this, &ClgcSettingsWidget::readRequested, clgcTx2RelThreshWidget, &IIOWidget::readAsync);
	}

	// CLGC TX1 Rel Threshold Enable - CheckBoxUi
	auto clgcTx1RelThreshEnWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,clgc-tx1-rel-threshold-en", "CLGC TX1 Rel Threshold Enable", this);
	if(clgcTx1RelThreshEnWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(clgcTx1RelThreshEnWidget);
		layout->addWidget(clgcTx1RelThreshEnWidget);
		m_widgets.append(clgcTx1RelThreshEnWidget);
		connect(this, &ClgcSettingsWidget::readRequested, clgcTx1RelThreshEnWidget, &IIOWidget::readAsync);
	}

	// CLGC TX2 Rel Threshold Enable - CheckBoxUi
	auto clgcTx2RelThreshEnWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,clgc-tx2-rel-threshold-en", "CLGC TX2 Rel Threshold Enable", this);
	if(clgcTx2RelThreshEnWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(clgcTx2RelThreshEnWidget);
		layout->addWidget(clgcTx2RelThreshEnWidget);
		m_widgets.append(clgcTx2RelThreshEnWidget);
		connect(this, &ClgcSettingsWidget::readRequested, clgcTx2RelThreshEnWidget, &IIOWidget::readAsync);
	}

	return section;
}
